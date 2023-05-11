#include <atomic>
#include <iostream>
#include <thread>

std::atomic<bool> ab1, ab2;
std::atomic<int> ai;

void write_ab1() { ab1.store(true, std::memory_order_release); }
void write_ab2() { ab2.store(true, std::memory_order_release); }

void read_ab1_then_ab2() {
    int counter = 0;
    
    while (!ab1.load(std::memory_order_acquire)) { ++counter; }
    if (ab2.load(std::memory_order_acquire)) { ++ai; }
    
    std::cout << "(read_ab1_then_ab2) counter = " << counter << '\n';
    std::cout << "(read_ab1_then_ab2)      ai = " << ai << '\n';
}

void read_ab2_then_ab1() {
    int counter = 0;
    
    while (!ab2.load(std::memory_order_acquire)) { ++counter; }
    if (ab1.load(std::memory_order_acquire)) { ++ai; }
    
    std::cout << "(read_ab2_then_ab1) counter = " << counter << '\n';
    std::cout << "(read_ab2_then_ab1)      ai = " << ai << '\n';
}

int main()
{
    ab1 = false, ab2 = false;
    ai = 0;
    
    std::thread t1(write_ab1);
    std::thread t2(write_ab2);
    std::thread t3(read_ab1_then_ab2);
    std::thread t4(read_ab2_then_ab1);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    assert(ai.load() != 0); // can fire like in relaxed-ordering
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// actual output
// (read_ab1_then_ab2) counter = (read_ab2_then_ab1) counter = 0
// (read_ab1_then_ab2)      ai = 0
// (read_ab2_then_ab1)      ai = 2
// 2
// Program ended with exit code: 0

// inferred output
// (read_ab2_then_ab1) counter = 0
// (read_ab1_then_ab2)      ai = 0
// (read_ab2_then_ab1)      ai = 2
// (read_ab1_then_ab2) counter = 2
// Program ended with exit code: 0
