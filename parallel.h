#include <algorithm>
#include <future>

namespace ABParallel {

// Parallel version of std::transform

template <typename srcIt, typename dstIt, typename functor>
auto par_transform(srcIt first, srcIt last, dstIt dst, functor func, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::transform(first, last, dst, func);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] {
        par_transform(first, srcMiddle, dst, func, chunkSize);
    });

    // Recursively handle the second part
    const auto dstMiddle = std::next(dst, n / 2);
    par_transform(srcMiddle, last, dstMiddle, func, chunkSize);
    future.wait();
}

// Parallel version of std::for_each

template <typename srcIt, typename functor>
auto par_for_each(srcIt first, srcIt last, functor func, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::for_each(first, last, func);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] {
        par_for_each(first, srcMiddle, func, chunkSize);
    });

    // Recursively handle the second part
    par_for_each(srcMiddle, last, func, chunkSize);
    future.wait();
}

// Parallel version of std::find

template <typename srcIt, typename valueType>
auto par_find(srcIt first, srcIt last, const valueType& value, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::find(first, last, value);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] () -> srcIt {
        return par_find(first, srcMiddle, value, chunkSize);
    });

    // Recursively handle the second part
    auto itFoundBySecondPart=par_find(srcMiddle, last, value, chunkSize);
    auto itFoundByFirstPart=future.get();

    // Return the first element found in the container
    if(itFoundByFirstPart!=srcMiddle)
        return itFoundByFirstPart;
    if(itFoundByFirstPart==srcMiddle && itFoundBySecondPart!=last)
        return itFoundBySecondPart;
    return last;
}

// Parallel version of std::find_if

template <typename srcIt, typename functor>
auto par_find_if(srcIt first, srcIt last, functor func, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::find_if(first, last, func);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_find_if(first, srcMiddle, func, chunkSize);
    });

    // Recursively handle the second part
    auto itFoundBySecondPart=par_find_if(srcMiddle, last, func, chunkSize);
    auto itFoundByFirstPart=future.get();

    // Return the first element found in the container
    if(itFoundByFirstPart!=srcMiddle)
        return itFoundByFirstPart;
    if(itFoundByFirstPart==srcMiddle && itFoundBySecondPart!=last)
        return itFoundBySecondPart;
    return last;
}

// Parallel version of std::fill

template <typename srcIt, typename valueType>
auto par_fill(srcIt first, srcIt last, const valueType& value, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::fill(first, last, value);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] {
        par_fill(first, srcMiddle, value, chunkSize);
    });

    // Recursively handle the second part
    par_fill(srcMiddle, last, value, chunkSize);
    future.wait();
}

// Parallel version of std::replace

template <typename srcIt, typename valueType>
auto par_replace(srcIt first, srcIt last, const valueType& oldValue, const valueType& newValue, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::replace(first, last, oldValue, newValue);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] {
        par_replace(first, srcMiddle, oldValue, newValue, chunkSize);
    });

    // Recursively handle the second part
    par_replace(srcMiddle, last, oldValue, newValue, chunkSize);
    future.wait();
}

// Parallel version of std::replace_if

template <typename srcIt, typename functor, typename valueType>
auto par_replace_if(srcIt first, srcIt last,  functor func, const valueType& newValue, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::replace_if(first, last, func, newValue);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] {
        par_replace_if(first, srcMiddle, func, newValue, chunkSize);
    });

    // Recursively handle the second part
    par_replace_if(srcMiddle, last, func, newValue, chunkSize);
    future.wait();
}

// Parallel version of std::copy

template <typename srcIt, typename dstIt>
auto par_copy(srcIt first, srcIt last, dstIt dst, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::copy(first, last, dst);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] {
        par_copy(first, srcMiddle, dst, chunkSize);
    });

    // Recursively handle the second part
    const auto dstMiddle = std::next(dst, n / 2);
    par_copy(srcMiddle, last, dstMiddle, chunkSize);
    future.wait();
}

// Parallel version of std::copy_if

template <typename srcIt, typename dstIt, typename functor>
auto par_copy_if(srcIt first, srcIt last, dstIt dst, functor func, size_t chunkSize) -> dstIt {
    auto n = static_cast<size_t>(std::distance(first, last));
    using copiedRange = std::pair<dstIt, dstIt>;
    using futureType = std::future< copiedRange >;
    auto futures = std::vector<futureType>{};
    futures.reserve(n / chunkSize);

    for (size_t startId = 0; startId < n; startId += chunkSize) {
        const auto stopId = std::min(startId + chunkSize, n);
        auto future = std::async(std::launch::async, [=, &func] {
            auto dstFirst = dst + startId;
            auto dstLast = std::copy_if(first + startId, first + stopId,
                                        dstFirst, func);
            return std::make_pair(dstFirst, dstLast);
        });
        futures.emplace_back(std::move(future));
    }

    auto dstLast = futures.front().get().second;
    for (auto it = std::next(futures.begin()); it != futures.end(); ++it) {
        auto chunkRange = it->get();
        dstLast = std::move(chunkRange.first, chunkRange.second, dstLast);
    }
    return dstLast;
}

// Parallel version of std::remove

template <typename srcIt, typename valueType>
auto par_remove(srcIt first, srcIt last, const valueType& value, size_t chunkSize) -> srcIt {
    auto n = static_cast<size_t>(std::distance(first, last));
    using analysedRange = std::pair<srcIt, srcIt>;
    using futureType = std::future< analysedRange >;
    auto futures = std::vector<futureType>{};
    futures.reserve(n / chunkSize);

    for (size_t startId = 0; startId < n; startId += chunkSize) {
        const auto stopId = std::min(startId + chunkSize, n);
        auto future = std::async(std::launch::async, [=] {
            auto itFound = std::remove(first + startId, first + stopId, value);
            return std::make_pair(first + startId, itFound);
        });
        futures.emplace_back(std::move(future));
    }

    auto dstLast = futures.front().get().second;
    for (auto it = std::next(futures.begin()); it != futures.end(); ++it) {
        auto chunkRange = it->get();
        dstLast = std::move(chunkRange.first, chunkRange.second, dstLast);
    }

    return dstLast;
}

// Parallel version of std::remove_if

template <typename srcIt, typename functor>
auto par_remove_if(srcIt first, srcIt last, functor func, size_t chunkSize) -> srcIt {
    auto n = static_cast<size_t>(std::distance(first, last));
    using analysedRange = std::pair<srcIt, srcIt>;
    using futureType = std::future< analysedRange >;
    auto futures = std::vector<futureType>{};
    futures.reserve(n / chunkSize);

    for (size_t startId = 0; startId < n; startId += chunkSize) {
        const auto stopId = std::min(startId + chunkSize, n);
        auto future = std::async(std::launch::async, [=, &func] {
            auto itFound = std::remove_if(first + startId, first + stopId, func);
            return std::make_pair(first + startId, itFound);
        });
        futures.emplace_back(std::move(future));
    }

    auto dstLast = futures.front().get().second;
    for (auto it = std::next(futures.begin()); it != futures.end(); ++it) {
        auto chunkRange = it->get();
        dstLast = std::move(chunkRange.first, chunkRange.second, dstLast);
    }

    return dstLast;
}

// Parallel version of std::count

template <typename srcIt, typename valueType>
auto par_count(srcIt first, srcIt last, const valueType& value, size_t chunkSize) -> typename std::iterator_traits<srcIt>::difference_type {
    using counterType=typename std::iterator_traits<srcIt>::difference_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::count(first, last, value);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] () -> counterType {
        return par_count(first, srcMiddle, value, chunkSize);
    });

    // Recursively handle the second part
    auto count1=par_count(srcMiddle, last, value, chunkSize);
    auto count2=future.get();
    return count1+count2;
}

// Parallel version of std::count_if

template <typename srcIt, typename functor>
auto par_count_if(srcIt first, srcIt last, functor func, size_t chunkSize) -> typename std::iterator_traits<srcIt>::difference_type {
    using counterType=typename std::iterator_traits<srcIt>::difference_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::count_if(first, last, func);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] () -> counterType {
        return par_count_if(first, srcMiddle, func, chunkSize);
    });

    // Recursively handle the second part
    auto count1=par_count_if(srcMiddle, last, func, chunkSize);
    auto count2=future.get();
    return count1+count2;
}

// Parallel version of std::max_element

template <typename srcIt>
auto par_max_element(srcIt first, srcIt last, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::max_element(first, last);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] () -> srcIt {
        return par_max_element(first, srcMiddle, chunkSize);
    });

    // Recursively handle the second part
    auto max1=par_max_element(srcMiddle, last, chunkSize);
    auto max2=future.get();
    return (*max2<*max1)?max1:max2;
}

template <typename srcIt, typename functor>
auto par_max_element(srcIt first, srcIt last, functor func, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::max_element(first, last, func);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_max_element(first, srcMiddle, func, chunkSize);
    });

    // Recursively handle the second part
    auto max1=par_max_element(srcMiddle, last, func, chunkSize);
    auto max2=future.get();
    return func(*max2,*max1)?max1:max2;
}

// Parallel version of std::min_element

template <typename srcIt>
auto par_min_element(srcIt first, srcIt last, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::min_element(first, last);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] () -> srcIt {
        return par_min_element(first, srcMiddle, chunkSize);
    });

    // Recursively handle the second part
    auto min1=par_min_element(srcMiddle, last, chunkSize);
    auto min2=future.get();
    return (*min2<=*min1)?min2:min1;
}

template <typename srcIt, typename functor>
auto par_min_element(srcIt first, srcIt last, functor func, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::min_element(first, last, func);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_min_element(first, srcMiddle, func, chunkSize);
    });

    // Recursively handle the second part
    auto min1=par_min_element(srcMiddle, last, func, chunkSize);
    auto min2=future.get();
    return func(*min2,*min1)?min2:min1;
}

// Parallel algorithm that calculates the sum of elements inside a container

template <typename srcIt>
auto par_sum(srcIt first, srcIt last, size_t chunkSize) -> typename std::iterator_traits<srcIt>::value_type {
    using valueType=typename std::iterator_traits<srcIt>::value_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::accumulate(first, last, static_cast<valueType>(0));
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=] () -> valueType {
        return par_sum(first, srcMiddle, chunkSize);
    });

    // Recursively handle the second part
    auto acc1=par_sum(srcMiddle, last, chunkSize);
    auto acc2=future.get();
    return acc1+acc2;
}

// Parallel algorithm that calculates the sum of elements inside a container after applying a functor to each element

template <typename srcIt, typename functor>
auto par_sum(srcIt first, srcIt last, functor func, size_t chunkSize) -> typename std::iterator_traits<srcIt>::value_type {
    using valueType=typename std::iterator_traits<srcIt>::value_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        valueType sum(0);
        for(srcIt i=first; i!=last; i=std::next(i))
            sum+=func(*i);
        return sum;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Branch of first part to another task
    auto future = std::async(std::launch::async, [=,&func] () -> valueType {
        return par_sum(first, srcMiddle, func, chunkSize);
    });

    // Recursively handle the second part
    auto acc1=par_sum(srcMiddle, last, func, chunkSize);
    auto acc2=future.get();
    return acc1+acc2;
}

// TO DO: generate, equal, all_of, any_of

}
