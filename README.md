# ABParallel
ABParallel is a C++ library that provides parallel algorithms supported by C++11 compilers. The algorithms have a similar syntax to that of the STL and uses the "divide and conquer" approach for most implementations.

The current supported algorithms are listed below:
* par_transform
* par_for_each
* par_generate
* par_fill
* par_sum
* par_count
* par_count_if
* par_copy
* par_copy_if
* par_find
* par_find_if
* par_find_if_not
* par_replace
* par_replace_if
* par_remove_if
* par_equal
* par_equal
* par_all_of
* par_any_of
* par_none_of
* par_max_element
* par_max_element

## Syntax
The syntax builds upon the one used in STL algorithms where container iterators are provided as arguments along with optional functors or lambdas.
The user needs also to provide an extra argument: the chunk size. This is the size of container elements that will be handled by a single task. Most of the time, the optimal chunke size is equal to the total elements of the container divided by the logical cores on the machine. However, this is not always the case and a sensitivity analysis for various chunk sizes might be useful to determine the best chunk size (check main.cpp for example).

## Installation
Using the algorithms of ABParallel is straightforward. Just include parallel.h in your project and use the namespace ABParallel.

## Examples
```c++
auto generateLambda = [](){
    auto min(0);
    auto max(500000);
    return  int(min + (rand() % static_cast<int>(max - min + 1)));
};

auto transformLambda = [](int a) {
    return sqrt(a);
};

auto unaryLambda = [](int a){
    return sqrt(sqrt(a))>3;
};

auto main() -> int{

    auto vectorSize=10000000;
    auto chunkSize = 1000000;

    std::vector<int> container(vectorSize);

    // generate random values for the container elements
    ABParallel::par_generate(container.begin(), container.end() , generateLambda, chunkSize);

    // transform each element of the container to its square root
    ABParallel::par_transform(container.begin(), container.end() , container.begin(), transformLambda, chunkSize);

    // calculate the sum of all the container elements
    auto sum = ABParallel::par_sum(container.begin(), container.end() , chunkSize);

    // count the number of elements that satisfy a certain condition
    auto count = ABParallel::par_sum(container.begin(), container.end() , unaryLambda, chunkSize);

    // find the first occurence of an element that satisfies a certain condition
    auto foundIterator = ABParallel::par_find_if(container.begin(), container.end() , unaryLambda, chunkSize);
}

```

## Notes
1. The work on this library is still in progress and some algorithms are not as generic as the STL equivalent ones.
2. Type requirements for the container iterators are similar to those used in the STL library.
3. Using par_remove_if is only recommended when followed by erase: the contents of the container after the returned iterator are in an undefined state.
4. par_sum is similar to std::accumulate when no lambda is used. Otherwise, par_sum calculates the sum of elements inside a container after applying the lambda to each element.


