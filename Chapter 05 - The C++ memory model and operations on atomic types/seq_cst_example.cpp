#include <atomic>
#include <vector>
#include <thread>
#include <iostream>

std::atomic<bool> ab1, ab2;
std::atomic<int> ai;

void write_ab1() {
    ab1.store(true, std::memory_order_seq_cst);
}

void write_ab2() {
    ab2.store(true, std::memory_order_seq_cst);
}

void read_ab1_then_ab2() {
    int counter = 0;
    while (!ab1.load(std::memory_order_seq_cst)) { ++counter; }
    if (ab2.load(std::memory_order_seq_cst)) { ++ai; }
    
    std::cout << "(read_ab1_then_ab2) counter = " << counter << '\n';
}

void read_ab2_then_ab1() {
    int counter = 0;
    while (!ab2.load(std::memory_order_seq_cst)) { ++counter; }
    if (ab1.load(std::memory_order_seq_cst)) { ++ai; }
    
    std::cout << "(read_ab2_then_ab1) counter = " << counter << '\n';
}

int main()
{
    ab1 = false;
    ab2 = false;
    
    ai = 0;
    
    std::thread t1(write_ab1);
    std::thread t2(write_ab2);
    std::thread t3(read_ab1_then_ab2);
    std::thread t4(read_ab2_then_ab1);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    assert(ai.load() != 0);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (read_ab2_then_ab1) counter = 0
// (read_ab1_then_ab2) counter = 134
// Program ended with exit code: 0
