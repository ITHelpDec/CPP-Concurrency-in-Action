#include <list>
#include <future>
#include <iostream>

void printList(const std::list<int> &ilist) {
    for (auto &&e: ilist) {
        std::cout << e << ' ';
    } std::cout << '\n';
}

namespace par {

template <typename T>
std::list<T> quick_sort(std::list<T> input) {
    if (input.empty()) { return input; }
    
    std::list<T> result;
    
    result.splice(result.begin(), input, input.begin());
    
    const T &pivot = *(result.begin());
    
    auto divide_point = std::partition(input.begin(), input.end(), [&] (const T &t) { return t < pivot; } );
    
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // only change from sequential function - sort lower portion on another thread
    std::future<std::list<T>> new_lower(std::async(&quick_sort<T>, std::move(lower_part)));
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    auto new_higher = quick_sort(std::move(input));
    
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get()); // .get() for future
    
    return result;
}

} // namespace par

int main()
{
    std::list<int> ilist = { 5, 3, 6, 8, 5, 4, 3 };
    
    std::cout << "Before: ";
    printList(ilist);
    
    ilist = par::quick_sort(ilist);
    
    std::cout << "After:  ";
    printList(ilist);
    
    return 0;
}
