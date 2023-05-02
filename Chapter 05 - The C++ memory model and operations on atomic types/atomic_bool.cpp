#include <atomic>
#include <iostream>

int main()
{
    std::cout << std::boolalpha;
    
    std::atomic<bool> b;
    std::cout << "initialising b...\n";
    std::cout << "b: " << b << "\n\n";
    
    bool x = b.load(std::memory_order_acquire);
    std::cout << "Loading b into x...\n";
    std::cout << "b: " << b << '\n';
    std::cout << "x: " << x << "\n\n";
    
    b.store(true);
    std::cout << "Storing true into b...\n";
    std::cout << "b: " << b << '\n';
    std::cout << "x: " << x << "\n\n";
    
    b.exchange(false, std::memory_order_acq_rel);
    std::cout << "Exchanging b's current value of true with false...\n";
    std::cout << "b: " << b << '\n';
    std::cout << "x: " << x << '\n';
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// initialising b...
// b: false

// Loading b into x...
// b: false
// x: false

// Storing true into b...
// b: true
// x: false

// Exchanging b's current value of true with false...
// b: false
// x: false
// Program ended with exit code: 0
