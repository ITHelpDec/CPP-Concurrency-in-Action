#include <list>
#include <iostream>

template <typename C>
void printContainer(const C &c) {
    for (auto &&e : c) {
        std::cout << e << ' ';
    } std::cout << '\n';
}

void hyphens(int layer) {
    for (int i = 0; i != layer; ++i) {
        std::cout << "- ";
    } std::cout << "[" << layer << "] ";
}

namespace std
{
template <typename T>
std::list<T> quick_sort(std::list<T> input, int layer) {
    if (input.empty()) { return input; }
    
    std::list<T> result;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // splice(a, b, c) -> transfer c from b before a
    result.splice(result.begin(), input, input.begin());
    
    hyphens(layer);
    std::cout << "result: ";
    printContainer(result);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // refer to pivot to avoid copy
    const T &pivot = *(result.begin());
    
    hyphens(layer);
    std::cout << "pivot: " << pivot << '\n';
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // split em up; less than pivot to the left; more than pivot to the right
    auto divide_point = std::partition(input.begin(), input.end(), [&] (const T &t) { return t < pivot; } );
    
    hyphens(layer);
    std::cout << "divide_point: " << *divide_point << '\n';
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::list<T> lower_part;
    
    // splice(a, b, c, d) -> transfer range (c, d] from b to before a
    lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);
    
    hyphens(layer);
    std::cout << "lower_part: ";
    printContainer(lower_part);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // recursively call on new lower and upper parts
    auto new_lower = quick_sort(std::move(lower_part), layer + 1);
    auto new_higher = quick_sort(std::move(input), layer + 1);
    
    hyphens(layer);
    std::cout << "new_lower: ";
    printContainer(new_lower);
    
    hyphens(layer);
    std::cout << "new_higher: ";
    printContainer(new_higher);
    
    hyphens(layer);
    std::cout << "result: ";
    printContainer(result);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // splice(a, b) -> transfer b before a
    result.splice(result.end(), new_higher);
    
    hyphens(layer);
    std::cout << "result with new_higher: ";
    printContainer(result);
    
    result.splice(result.begin(), new_lower);
    
    hyphens(layer);
    std::cout << "result with new_lower: ";
    printContainer(result);
    
    return result;
}
} // namespace std

int main()
{
    std::list<int> ilist = { 5, 3, 6, 8, 5, 4, 3 };
    
    std::cout << "Before: ";
    printContainer(ilist);
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "- - -\n";
    ilist = std::quick_sort(ilist, 0);
    std::cout << "- - -\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "After:  ";
    printContainer(ilist);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Before: 5 3 6 8 5 4 3
// - - -
// [0] result: 5
// [0] pivot: 5
// [0] divide_point: 5
// [0] lower_part: 3 3 4
// - [1] result: 3
// - [1] pivot: 3
// - [1] divide_point: 3
// - [1] lower_part:
// - - [2] result: 3
// - - [2] pivot: 3
// - - [2] divide_point: 4
// - - [2] lower_part:
// - - - [3] result: 4
// - - - [3] pivot: 4
// - - - [3] divide_point: 0
// - - - [3] lower_part:
// - - - [3] new_lower:
// - - - [3] new_higher:
// - - - [3] result: 4
// - - - [3] result with new_higher: 4
// - - - [3] result with new_lower: 4
// - - [2] new_lower:
// - - [2] new_higher: 4
// - - [2] result: 3
// - - [2] result with new_higher: 3 4
// - - [2] result with new_lower: 3 4
// - [1] new_lower:
// - [1] new_higher: 3 4
// - [1] result: 3
// - [1] result with new_higher: 3 3 4
// - [1] result with new_lower: 3 3 4
// - [1] result: 5
// - [1] pivot: 5
// - [1] divide_point: 8
// - [1] lower_part:
// - - [2] result: 8
// - - [2] pivot: 8
// - - [2] divide_point: 1
// - - [2] lower_part: 6
// - - - [3] result: 6
// - - - [3] pivot: 6
// - - - [3] divide_point: 0
// - - - [3] lower_part:
// - - - [3] new_lower:
// - - - [3] new_higher:
// - - - [3] result: 6
// - - - [3] result with new_higher: 6
// - - - [3] result with new_lower: 6
// - - [2] new_lower: 6
// - - [2] new_higher:
// - - [2] result: 8
// - - [2] result with new_higher: 8
// - - [2] result with new_lower: 6 8
// - [1] new_lower:
// - [1] new_higher: 6 8
// - [1] result: 5
// - [1] result with new_higher: 5 6 8
// - [1] result with new_lower: 5 6 8
// [0] new_lower: 3 3 4
// [0] new_higher: 5 6 8
// [0] result: 5
// [0] result with new_higher: 5 5 6 8
// [0] result with new_lower: 3 3 4 5 5 6 8
// - - -
// After:  3 3 4 5 5 6 8
// Program ended with exit code: 0
