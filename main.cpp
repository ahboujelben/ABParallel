#include <iostream>
#include <vector>
#include <chrono>

#include "parallel.h"

auto main() -> int
{
    using namespace ABParallel;
    using ClockType = std::chrono::steady_clock;

    const auto n = size_t{100000000};
    auto src = std::vector<int>(n);
    std::generate(src.begin(), src.end(), []() {
    return int(std::rand());
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
        auto f = par_count_if(src.begin(), src.end(), [](int val){return val%7==0;}, chunk_sz);

        auto stop = ClockType::now();
        auto duration = (stop - start_);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << f<<" "<< chunk_sz<<": "<< ms << " ms " << '\n';
    }


}
