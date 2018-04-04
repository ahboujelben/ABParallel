#include <iostream>
#include <future>
#include <vector>
#include <chrono>
#include <algorithm>

template <typename SrcIt, typename DstIt, typename Func>
auto par_transform(SrcIt first, SrcIt last, DstIt dst, Func func, size_t chunk_sz) -> void {
  const auto n = static_cast<size_t>(std::distance(first, last));
  if (n <= chunk_sz) {
    std::transform(first, last, dst, func);
    return;
  }
  const auto src_middle = std::next(first, n / 2);

  // Branch of first part to another task
  auto future = std::async(std::launch::async, [=, &func] {
    par_transform(first, src_middle, dst, func, chunk_sz);
  });

  // Recursively handle the second part
  const auto dst_middle = std::next(dst, n / 2);
  par_transform(src_middle, last, dst_middle, func, chunk_sz);
  future.wait();
}

template <typename SrcIt, typename Func>
auto par_for_each(SrcIt first, SrcIt last, Func func, size_t chunk_sz) -> void {
  const auto n = static_cast<size_t>(std::distance(first, last));
  if (n <= chunk_sz) {
    std::for_each(first, last, func);
    return;
  }
  const auto src_middle = std::next(first, n / 2);

  // Branch of first part to another task
  auto future = std::async(std::launch::async, [=, &func] {
    par_for_each(first, src_middle, func, chunk_sz);
  });

  // Recursively handle the second part
  par_for_each(src_middle, last, func, chunk_sz);
  future.wait();
}

template <typename SrcIt, typename DstIt, typename Func>
auto par_copy_if(SrcIt first, SrcIt last, DstIt dst, Func func, size_t chunk_sz) -> DstIt {
  auto n = static_cast<size_t>(std::distance(first, last));
  using CopiedRange = std::pair<DstIt, DstIt>;
  using FutureType = std::future< CopiedRange >;
  auto futures = std::vector<FutureType>{};
  futures.reserve(n / chunk_sz);

  for (size_t start_idx = 0; start_idx < n; start_idx += chunk_sz) {
    const auto stop_idx = std::min(start_idx + chunk_sz, n);
    auto future = std::async(std::launch::async, [=, &func] {
      auto dst_first = dst + start_idx;
      auto dst_last = std::copy_if(first + start_idx, first + stop_idx,
      dst_first, func);
      return std::make_pair(dst_first, dst_last);
    });
    futures.emplace_back(std::move(future));
  }

  auto new_end = futures.front().get().second;
  for (auto it = std::next(futures.begin()); it != futures.end(); ++it) {
    auto chunk_range = it->get();
    new_end = std::move(chunk_range.first, chunk_range.second, new_end);
  }
  return new_end;
}

template <typename SrcIt, typename Func>
auto par_max_element(SrcIt first, SrcIt last, Func func, size_t chunk_sz) -> SrcIt {
  const auto n = static_cast<size_t>(std::distance(first, last));
  if (n <= chunk_sz) {
    return std::max_element(first, last, func);
  }
  const auto src_middle = std::next(first, n / 2);

  // Branch of first part to another task
  auto future = std::async(std::launch::async, [=, &func] () -> SrcIt {
    return par_max_element(first, src_middle, func, chunk_sz);
  });

  // Recursively handle the second part
  auto max1=par_max_element(src_middle, last, func, chunk_sz);
  auto max2=future.get();
  return func(*max2,*max1)?max1:max2;
}

template <typename SrcIt>
auto par_max_element(SrcIt first, SrcIt last, size_t chunk_sz) -> SrcIt {
  const auto n = static_cast<size_t>(std::distance(first, last));
  if (n <= chunk_sz) {
    return std::max_element(first, last);
  }
  const auto src_middle = std::next(first, n / 2);

  // Branch of first part to another task
  auto future = std::async(std::launch::async, [=] () -> SrcIt {
      return par_max_element(first, src_middle, chunk_sz);
  });

  // Recursively handle the second part
  auto max1=par_max_element(src_middle, last, chunk_sz);
  auto max2=future.get();
  return (*max2<*max1)?max1:max2;
}

template <typename SrcIt>
auto par_sum(SrcIt first, SrcIt last, size_t chunk_sz) -> typename SrcIt::value_type {
    using val_type=typename SrcIt::value_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunk_sz) {
    return std::accumulate(first, last, static_cast<val_type>(0));
    }
    const auto src_middle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] () -> val_type {
      return par_sum(first, src_middle, chunk_sz);
    });

    // Recursively handle the second part
    auto acc1=par_sum(src_middle, last, chunk_sz);
    auto acc2=future.get();
    return acc1+acc2;
}

template <typename SrcIt, typename Func>
auto par_sum(SrcIt first, SrcIt last, Func func, size_t chunk_sz) -> typename SrcIt::value_type {
    using val_type=typename SrcIt::value_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunk_sz) {
        val_type sum(0);
        for(SrcIt i=first; i!=last; i=std::next(i))
            sum+=func(*i);
        return sum;
    }
    const auto src_middle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=,&func] () -> val_type {
      return par_sum(first, src_middle, func, chunk_sz);
    });

    // Recursively handle the second part
    auto acc1=par_sum(src_middle, last, func, chunk_sz);
    auto acc2=future.get();
    return acc1+acc2;
}

auto main() -> int
{
    using ClockType = std::chrono::steady_clock;

    const auto n = size_t{100000000};
    auto src = std::vector<float>(n);
    std::generate(src.begin(), src.end(), []() {
    return float(std::rand());
    });

    //auto dst = std::vector<float>(n);

    auto transform_func = [](float v) {
    auto sum = v;
    for (size_t i = 0; i < 10; ++i) {
    sum += sqrt(i*i*i*sum);
    }
    return sum;
    };

    auto comp_func = [](float a, float b){
        return sqrt(sqrt(sqrt(a)))<sqrt(sqrt(sqrt(b)));
    };

    auto add_func = [](float a, float b){
        return a+sqrt(sqrt(sqrt(b)));
    };

    std::vector<size_t> sizes{1000000, 5000000, 10000000, 20000000, 25000000, 50000000, 100000000};

    for(size_t chunk_sz:sizes)
    {
        const ClockType::time_point start_ = ClockType::now();

        //par_for_each(src.begin(), src.end(), transform_func, chunk_sz);
        auto f = par_sum(src.begin(), src.end(), chunk_sz);

        auto stop = ClockType::now();
        auto duration = (stop - start_);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << f<<" "<< chunk_sz<<": "<< ms << " ms " << '\n';
    }


}
