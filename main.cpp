#include <iostream>
#include <vector>
#include <chrono>
#include <cassert>

#include "include\parallel.h"

#define PRINT_FUNC() std::cout << __func__ << " ";

auto generateTestContainer() -> std::vector<int>{
    const auto n = size_t{100000000};
    auto src = std::vector<int>(n);
    std::generate(src.begin(), src.end(), []() {
        auto min(0);
        auto max(500000);
        return  min + (rand() % static_cast<int>(max - min + 1));
    });
    return src;
}

//This function measures the performance of a parallel algorithm running on a container for different chunk sizes (if the chunk size is greater than the container size, the STL equivalent algorithm is used)

template<typename container>
auto testAlgorithmPerformance(container testContainer, std::size_t chunkSize, void (*testedAlgorithm) (container&, std::size_t)) -> void {

    using timer = std::chrono::steady_clock;
    const timer::time_point start = timer::now();

    testedAlgorithm(testContainer, chunkSize);

    auto stop = timer::now();
    auto duration = (stop - start);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    std::cout <<"Chunk size: "<<chunkSize<<" / Processing time: "<< ms << " ms " << '\n';
}

//Lambdas used for testing

auto transformLambda = [](int a) {
    return sqrt(sqrt(a));
};

auto compareLambda = [](int a, int b){
    return sqrt(sqrt(a))<sqrt(sqrt(b));
};

auto unaryLambda = [](int a){
    return abs(sqrt(sqrt(sqrt(a))))<0;
};

auto generateLambda = [](){
    auto min(0);
    auto max(500000);
    return  int(min + (rand() % static_cast<int>(max - min + 1)));
};

///////////// Testing the performance of few algorithms

//Testing par_transform

auto vector_par_transform(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_transform(src.begin(), src.end(), src.begin(), transformLambda , chunkSize);
}

//Testing par_for_each

auto vector_par_for_each(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_for_each(src.begin(), src.end(), transformLambda , chunkSize);
}

//Testing generate
auto vector_par_generate(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_generate(src.begin(), src.end() , generateLambda, chunkSize);
}


//Testing sum
auto vector_par_sum(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_sum(src.begin(), src.end(), chunkSize);
}

//Testing count
auto vector_par_count(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_count(src.begin(), src.end(), 250, chunkSize);
}

//Testing find_if
auto vector_par_find_if(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_find_if(src.begin(), src.end() , unaryLambda, chunkSize);
}

//Testing par_replace_if

auto vector_par_replace_if(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_replace_if(src.begin(), src.end(), unaryLambda, 14 , chunkSize);
}

//Testing par_remove_if

auto vector_par_remove_if(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    auto foundIt=ABParallel::par_remove_if(src.begin(), src.end(), unaryLambda , chunkSize);
    src.erase(foundIt, src.end());
}

//Testing none_of
auto vector_par_none_of(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_none_of(src.begin(), src.end() , unaryLambda, chunkSize);
}

//Testing max_element
auto vector_par_max_element(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_max_element(src.begin(), src.end(), chunkSize);
}

//Testing min_element
auto vector_par_min_element(std::vector<int>& src, std::size_t chunkSize) -> void{
    PRINT_FUNC();
    ABParallel::par_min_element(src.begin(), src.end(), chunkSize);
}

auto main() -> int{

    std::cout<<"Starting performance testing of few ABParallel algorithms. \n\nNote that the last chunk size corresponds to the sequential STL algorithm.\n\n";

    auto testContainer=generateTestContainer();

    std::vector<size_t> chunkSizes{1000000, 5000000, 10000000, 20000000, 25000000, 50000000, 100000000};

    std::vector<void (*) (std::vector<int>&, std::size_t)> testedAlgorithms{
        vector_par_transform,
        vector_par_for_each,
        vector_par_generate,
        vector_par_sum,
        vector_par_count,
        vector_par_find_if,
        vector_par_replace_if,
        vector_par_remove_if,
        vector_par_none_of,
        vector_par_max_element,
        vector_par_min_element
    };

    for(auto testedAlgorithm: testedAlgorithms){
        for(auto chunkSize: chunkSizes)
            testAlgorithmPerformance(testContainer, chunkSize, testedAlgorithm);
        std::cout<<std::endl;
    }
}
