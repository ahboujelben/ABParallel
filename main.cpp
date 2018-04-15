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
        auto min(0);
        auto max(50000);
        return  min + (rand() % static_cast<int>(max - min + 1));
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
        auto foundIt = par_find_if(src.begin(), src.end(), [](int a)->bool{return int(sqrt(sqrt(sqrt(sqrt(a)))))==13;}, chunk_sz);
        //auto foundInt = foundIt==src.end()?-99999:*foundIt;

        auto stop = ClockType::now();
        auto duration = (stop - start_);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout << foundIt-src.begin() <<" chunk: "<<chunk_sz<<": "<< ms << " ms " << '\n';
    }

    std::cout << find_if(src.begin(), src.end(), [](int a)->bool{return int(sqrt(sqrt(sqrt(sqrt(a)))))==13;})-src.begin() << '\n';

}
