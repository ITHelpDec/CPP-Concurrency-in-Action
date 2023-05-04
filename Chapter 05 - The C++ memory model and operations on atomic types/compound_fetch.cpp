#include <atomic>
#include <cassert>
#include <iostream>

class Foo { };

void printArray(Foo *arr, std::size_t sz) {
    for (int i = 0; i != sz; ++i) {
        std::cout << &arr[i] << ' ';
    } std::cout << '\n';
}

int main()
{
    Foo some_array[5];
    
    std::cout << "array: ";
    printArray(some_array, sizeof(some_array));
    
    std::atomic<Foo*> p(some_array);
    
    Foo *x = p.fetch_add(2);                // shift pointer forward two steps ([0] -> [2]) AFTER assignment
    std::cout << "    x: " << x << '\n';    // still points to head of array
    
    assert(x == some_array);
    assert(p.load() == &some_array[2]);
    
    x = p -= 1;                             // shift back one THEN assign
    std::cout << "    x: " << x << '\n';    // should now point to [1]
    
    assert(x == &some_array[1]);
    assert(p.load() == &some_array[1]);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// array: 0x16fdff1a7 0x16fdff1a8 0x16fdff1a9 0x16fdff1aa 0x16fdff1ab
//     x: 0x16fdff1a7
//     x: 0x16fdff1a8
// Program ended with exit code: 0
