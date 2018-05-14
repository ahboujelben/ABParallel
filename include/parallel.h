/////////////////////////////////////////////////////////////////////////////
/// Name:        ABParallel.h
/// Purpose:     A c++ library of parallel algorithms.
/// Author:      Ahmed Hamdi Boujelben <ahmed.hamdi.boujelben@gmail.com>
/// Created:     2018
/// Copyright:   (c) 2018 Ahmed Hamdi Boujelben
/// Licence:     Attribution-NonCommercial 4.0 International
/////////////////////////////////////////////////////////////////////////////

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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] {
        par_transform(first, srcMiddle, dst, func, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] {
        par_for_each(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    par_for_each(srcMiddle, last, func, chunkSize);
    future.wait();
}

// Parallel version of std::generate

template <typename srcIt, typename functor>
auto par_generate(srcIt first, srcIt last, functor func, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::generate(first, last, func);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] {
        par_generate(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    par_generate(srcMiddle, last, func, chunkSize);
    future.wait();
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] {
        par_fill(first, srcMiddle, value, chunkSize);
    });

    // Treat the second part recursively
    par_fill(srcMiddle, last, value, chunkSize);
    future.wait();
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] () -> valueType {
        return par_sum(first, srcMiddle, chunkSize);
    });

    // Treat the second part recursively
    auto acc1=par_sum(srcMiddle, last, chunkSize);

    // Collect the sum of both parts
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=,&func] () -> valueType {
        return par_sum(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    auto acc1=par_sum(srcMiddle, last, func, chunkSize);

    // Collect the sum of both parts
    auto acc2=future.get();
    return acc1+acc2;
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] () -> counterType {
        return par_count(first, srcMiddle, value, chunkSize);
    });

    // Treat the second part recursively
    auto count1=par_count(srcMiddle, last, value, chunkSize);

    // Collect the count of both parts
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] () -> counterType {
        return par_count_if(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    auto count1=par_count_if(srcMiddle, last, func, chunkSize);

    // Collect the count of both parts
    auto count2=future.get();
    return count1+count2;
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] {
        par_copy(first, srcMiddle, dst, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a table of futures to handle each chunk asynchronously
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

    // Adjust the destination container
    auto dstLast = futures.front().get().second;
    for (auto it = std::next(futures.begin()); it != futures.end(); ++it) {
        auto chunkRange = it->get();
        dstLast = std::move(chunkRange.first, chunkRange.second, dstLast);
    }
    return dstLast;
}

// Parallel version of std::find

template <typename srcIt, typename valueType>
auto par_find(srcIt first, srcIt last, const valueType& value, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::find(first, last, value);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] () -> srcIt {
        return par_find(first, srcMiddle, value, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_find_if(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    auto itFoundBySecondPart=par_find_if(srcMiddle, last, func, chunkSize);
    auto itFoundByFirstPart=future.get();

    // Return the first element found in the container
    if(itFoundByFirstPart!=srcMiddle)
        return itFoundByFirstPart;
    if(itFoundByFirstPart==srcMiddle && itFoundBySecondPart!=last)
        return itFoundBySecondPart;
    return last;
}

// Parallel version of std::find_if_not

template <typename srcIt, typename functor>
auto par_find_if_not(srcIt first, srcIt last, functor func, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::find_if_not(first, last, func);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_find_if_not(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    auto itFoundBySecondPart=par_find_if_not(srcMiddle, last, func, chunkSize);
    auto itFoundByFirstPart=future.get();

    // Return the first element found in the container
    if(itFoundByFirstPart!=srcMiddle)
        return itFoundByFirstPart;
    if(itFoundByFirstPart==srcMiddle && itFoundBySecondPart!=last)
        return itFoundBySecondPart;
    return last;
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] {
        par_replace(first, srcMiddle, oldValue, newValue, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] {
        par_replace_if(first, srcMiddle, func, newValue, chunkSize);
    });

    // Treat the second part recursively
    par_replace_if(srcMiddle, last, func, newValue, chunkSize);
    future.wait();
}

// Parallel version of std::remove_if
// Caution: all the elements stored after the returned iterator are in undefined state. This method
// is only recommended if followed by erase

template <typename srcIt, typename functor>
auto par_remove_if(srcIt first, srcIt last, functor func, size_t chunkSize) -> srcIt {
    auto n = static_cast<size_t>(std::distance(first, last));
    using analysedRange = std::pair<srcIt, srcIt>;
    using futureType = std::future< analysedRange >;
    auto futures = std::vector<futureType>{};
    futures.reserve(n / chunkSize);

    // Create a table of futures to handle each chunk asynchronously
    for (size_t startId = 0; startId < n; startId += chunkSize) {
        const auto stopId = std::min(startId + chunkSize, n);
        auto future = std::async(std::launch::async, [=, &func] {
            auto itFound = std::remove_if(first + startId, first + stopId, func);
            return std::make_pair(first + startId, itFound);
        });
        futures.emplace_back(std::move(future));
    }

    // Adjust the  container
    auto dstLast = futures.front().get().second;
    for (auto it = std::next(futures.begin()); it != futures.end(); ++it) {
        auto chunkRange = it->get();
        dstLast = std::move(chunkRange.first, chunkRange.second, dstLast);
    }

    return dstLast;
}

// Parallel version of std::sort

// Perform a merge of two sorted containers
template <typename srcIt, typename functor>
auto par_merge(srcIt first, srcIt middle, srcIt last,functor func) -> void {

    // Create two temporary vectors to store the current elements of both chunks
    const auto n1 = static_cast<size_t>(std::distance(first, middle));
    const auto n2 = static_cast<size_t>(std::distance(middle, last));
    using valueType = typename std::iterator_traits<srcIt>::value_type;
    std::vector<valueType> arrayLeft(n1), arrayRight(n2);
    std::copy(first, middle, arrayLeft.begin());
    std::copy(middle, last, arrayRight.begin());

    // Initialise indices
    auto helperLeft(arrayLeft.begin()), helperLeftEnd(arrayLeft.end()), helperRight(arrayRight.begin()), helperRightEnd(arrayRight.end());
    auto current(first);

    // Merge sort by comparing the elements of the temporary arrays
    while (helperLeft != helperLeftEnd && helperRight != helperRightEnd) {

        if(func(*helperLeft, *helperRight)){
            *current = *helperLeft;
            helperLeft=std::next(helperLeft);
        }

        else{
            *current = *helperRight;
            helperRight=std::next(helperRight);
        }

        current=std::next(current);
    }

    // Copy the remaining  elements of the left array
    while(helperLeft != helperLeftEnd){
        *current = *helperLeft;
        helperLeft=std::next(helperLeft);
        current=std::next(current);
    }
}

template <typename srcIt, typename functor>
auto par_sort(srcIt first, srcIt last,functor func, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::sort(first, last, func);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] {
        par_sort(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    par_sort(srcMiddle, last, func, chunkSize);
    future.wait();

    // Merge the two sorted parts
    par_merge(first, srcMiddle, last, func);
}

// Perform a merge of two sorted containers
template <typename srcIt>
auto par_merge(srcIt first, srcIt middle, srcIt last) -> void {

    // Create two temporary vectors to store the current elements of both chunks
    const auto n1 = static_cast<size_t>(std::distance(first, middle));
    const auto n2 = static_cast<size_t>(std::distance(middle, last));
    using valueType = typename std::iterator_traits<srcIt>::value_type;
    std::vector<valueType> arrayLeft(n1), arrayRight(n2);
    std::copy(first, middle, arrayLeft.begin());
    std::copy(middle, last, arrayRight.begin());

     // Initialise indices
    auto helperLeft(arrayLeft.begin()), helperLeftEnd(arrayLeft.end()), helperRight(arrayRight.begin()), helperRightEnd(arrayRight.end());
    auto current(first);

    // Merge sort by comparing the elements of the temporary arrays
    while (helperLeft != helperLeftEnd && helperRight != helperRightEnd) {

        if(*helperLeft <= *helperRight){
            *current = *helperLeft;
            helperLeft=std::next(helperLeft);
        }

        else{
            *current = *helperRight;
            helperRight=std::next(helperRight);
        }

        current=std::next(current);
    }

    // Copy the remaining  elements of the left array
    while(helperLeft != helperLeftEnd){
        *current = *helperLeft;
        helperLeft=std::next(helperLeft);
        current=std::next(current);
    }
}

template <typename srcIt>
auto par_sort(srcIt first, srcIt last, size_t chunkSize) -> void {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        std::sort(first, last);
        return;
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] {
        par_sort(first, srcMiddle, chunkSize);
    });

    // Treat the second part recursively
    par_sort(srcMiddle, last, chunkSize);
    future.wait();

    // Merge the two sorted parts
    par_merge(first, srcMiddle, last);
}

// Parallel version of equal

template <typename srcIt, typename dstIt, typename functor>
auto par_equal(srcIt first, srcIt last, dstIt dst, functor func, size_t chunkSize) -> bool {
    using valueType=typename std::iterator_traits<srcIt>::value_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::equal(first, last, dst, func);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=,&func] () -> valueType {
        return par_equal(first, srcMiddle, dst, func, chunkSize);
    });

    // Treat the second part recursively
    const auto dstMiddle = std::next(dst, n / 2);
    auto equal1=par_equal(srcMiddle, last, dstMiddle, func, chunkSize);

    auto equal2=future.get();
    return equal1 && equal2;
}

template <typename srcIt, typename dstIt>
auto par_equal(srcIt first, srcIt last, dstIt dst, size_t chunkSize) -> bool {
    using valueType=typename std::iterator_traits<srcIt>::value_type;
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::equal(first, last, dst);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] () -> valueType {
        return par_equal(first, srcMiddle, dst, chunkSize);
    });

    // Treat the second part recursively
    const auto dstMiddle = std::next(dst, n / 2);
    auto equal1=par_equal(srcMiddle, last, dstMiddle, chunkSize);

    auto equal2=future.get();
    return equal1 && equal2;
}

// Parallel version of all_of

template <typename srcIt, typename functor>
auto par_all_of(srcIt first, srcIt last, functor func, size_t chunkSize) -> bool {
    return par_find_if_not(first, last, func, chunkSize) == last;
}

// Parallel version of any_of

template <typename srcIt, typename functor>
auto par_any_of(srcIt first, srcIt last, functor func, size_t chunkSize) -> bool {
    return par_find_if(first, last, func, chunkSize) != last;
}

// Parallel version of none_of

template <typename srcIt, typename functor>
auto par_none_of(srcIt first, srcIt last, functor func, size_t chunkSize) -> bool {
    return par_find_if(first, last, func, chunkSize) == last;
}

// Parallel version of std::max_element

template <typename srcIt>
auto par_max_element(srcIt first, srcIt last, size_t chunkSize) -> srcIt {
    const auto n = static_cast<size_t>(std::distance(first, last));
    if (n <= chunkSize) {
        return std::max_element(first, last);
    }
    const auto srcMiddle = std::next(first, n / 2);

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] () -> srcIt {
        return par_max_element(first, srcMiddle, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_max_element(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=] () -> srcIt {
        return par_min_element(first, srcMiddle, chunkSize);
    });

    // Treat the second part recursively
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

    // Create a new task to treat the first part
    auto future = std::async(std::launch::async, [=, &func] () -> srcIt {
        return par_min_element(first, srcMiddle, func, chunkSize);
    });

    // Treat the second part recursively
    auto min1=par_min_element(srcMiddle, last, func, chunkSize);
    auto min2=future.get();
    return func(*min2,*min1)?min2:min1;
}

}
