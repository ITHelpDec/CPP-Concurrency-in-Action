#include <atomic>
#include <thread>
#include <iostream>

bool b;

std::atomic<bool> ab;
std::atomic<int> ai;

void write_b_then_ab() {
    b = true;
    std::atomic_thread_fence(std::memory_order_release);
    ab.store(true, std::memory_order_relaxed);
}

void read_ab_then_b() {
    int counter = 0;
    while (!ab.load(std::memory_order_relaxed)) { ++counter; }
    std::atomic_thread_fence(std::memory_order_acquire);
    if (b) { ++ai; }
    
    std::cout << "(read_ab_then_b) counter = " << counter << '\n';
    std::cout << "(read_ab_then_b) ai      = " << ai.load(std::memory_order_relaxed)<< '\n';
}

int main()
{
    b = false;
    ab = false;
    ai = 0;
    
    std::thread t1(write_b_then_ab);
    std::thread t2(read_ab_then_b);
    
    t1.join();
    t2.join();
    
    assert(ai.load() != 0);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (read_ab_then_b) counter = 0
// (read_ab_then_b) ai      = 1
// Program ended with exit code: 0
