#include <iostream>
#include <vector>
#include <chrono>
#include <cassert>

#include "parallel.h"

auto generateTestContainer() -> std::vector<int>{
    const auto n = size_t{100000000};
    auto src = std::vector<int>(n);
    std::generate(src.begin(), src.end(), []() {
        auto min(0);
        auto max(50000);
        return  min + (rand() % static_cast<int>(max - min + 1));
    });
    return src;
}

//This function measures the performance of a parallel algorithm running on a container for different chunk sizes (if the chunk size is greater than the container size, the STL equivalent algorithm is used)

template<typename container, typename chunkContainer>
auto testPerformanceForDifferentChunkSizes(void (*algorithmTester) (container,std::size_t),  const chunkContainer& sizes) -> void {
    auto testContainer=generateTestContainer();

    for(auto chunkSize:sizes)
    {
        container testContainerFreshCopy=testContainer;

        using timer = std::chrono::steady_clock;
        const timer::time_point start = timer::now();

        algorithmTester(testContainerFreshCopy, chunkSize);

        auto stop = timer::now();
        auto duration = (stop - start);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        std::cout <<"Chunk size: "<<chunkSize<<" / Processing time: "<< ms << " ms " << '\n';
    }
}

//This function tests the presence of data races by performing multiple runs and comparing the result of  the parallel algorithm to the sequential one

template<typename container>
auto testAccuracyForDifferentRuns(void (*algorithmTester) (container, container, std::size_t), std::size_t chunkSize) -> void {
    auto testContainer=generateTestContainer();
    for(auto i=0; i<20; i++)
    {
        container testContainerFreshCopyParallel=testContainer;
        container testContainerFreshCopySequential=testContainer;
        algorithmTester(testContainerFreshCopyParallel, testContainerFreshCopySequential, chunkSize);
    }
}

//Lambdas used for testing

auto transformLambda = [](int v) {
    auto sum = v;
    for (size_t i = 0; i < 10; ++i) {
        sum += sqrt(i*i*i*sum);
    }
    return sum;
};

auto compareLambda = [](int a, int b){
    return sqrt(sqrt(sqrt(a)))<sqrt(sqrt(sqrt(b)));
};

auto unaryLambda = [](int a){
    return sqrt(a)<20;
};

//Testing par_replace

auto testPerformace_par_replace(std::vector<int>& src, std::size_t chunkSize) -> void{
    ABParallel::par_replace(src.begin(), src.end(), 12, 14 , chunkSize);
}

auto testAccuracy_par_replace(std::vector<int>& testContainerParallel, std::vector<int>& testContainerSequential, std::size_t chunkSize) -> void{
    ABParallel::par_replace(testContainerParallel.begin(), testContainerParallel.end(), 12, 14 , chunkSize);
    std::replace(testContainerSequential.begin(), testContainerSequential.end(), 12, 14);
    assert(testContainerParallel==testContainerSequential && "Accuracy test failed.");
}

//Testing par_replace_if

auto testPerformace_par_replace_if(std::vector<int>& src, std::size_t chunkSize) -> void{
    ABParallel::par_replace_if(src.begin(), src.end(), unaryLambda, 14 , chunkSize);
}

auto testAccuracy_par_replace_if(std::vector<int>& testContainerParallel, std::vector<int>& testContainerSequential, std::size_t chunkSize) -> void{
    ABParallel::par_replace_if(testContainerParallel.begin(), testContainerParallel.end(), unaryLambda, 14 , chunkSize);
    std::replace_if(testContainerSequential.begin(), testContainerSequential.end(), unaryLambda, 14);
    assert(testContainerParallel==testContainerSequential && "Accuracy test failed.");
}

auto main() -> int{

    std::vector<size_t> chunkSizes{1000000, 5000000, 10000000, 20000000, 25000000, 50000000, 100000000};

    testPerformanceForDifferentChunkSizes(testPerformace_par_replace_if, chunkSizes);
    testAccuracyForDifferentRuns(testAccuracy_par_replace_if, 10000000);
}
