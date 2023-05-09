#include <atomic>
#include <iostream>
#include <thread>

std::atomic<bool> ab1, ab2;
std::atomic<int> ai;

void write_ab1_then_ab2() {
    ab1.store(true, std::memory_order_relaxed);
    ab2.store(true, std::memory_order_relaxed);
}

void read_ab2_then_ab1() {
    int counter = 0;
    
    while (!ab2.load(std::memory_order_relaxed)) { ++counter; }
    if (ab1.load(std::memory_order_relaxed)) { ++ai; }
    
    std::cout << "(read_ab2_then_ab1) counter = " << counter << '\n';
}

int main()
{
    ab1 = false;
    ab2 = false;
    
    ai = 0;
    
    std::thread t1(write_ab1_then_ab2);
    std::thread t2(read_ab2_then_ab1);
    
    t1.join();
    t2.join();
    
    assert(ai.load() != 0);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (read_ab2_then_ab1) counter = 0
// Program ended with exit code: 0
