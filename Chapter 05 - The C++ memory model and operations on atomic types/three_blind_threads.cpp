#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> data[5];

std::atomic<bool> sync1(false), sync2(false);

void thread_one() {
    data[0].store(0, std::memory_order_relaxed);
    data[1].store(1, std::memory_order_relaxed);
    data[2].store(2, std::memory_order_relaxed);
    data[3].store(3, std::memory_order_relaxed);
    data[4].store(4, std::memory_order_relaxed);
    
    sync1.store(true, std::memory_order_release); // end of "batch"
}

void thread_two() {
    int counter = 0;
    while (!sync1.load(std::memory_order_acquire)) { ++counter; }
    std::cout << "(thread_two)   counter = " << counter << '\n';
    
    sync2.store(true, std::memory_order_release);
}

void thread_three() {
    int counter = 0;
    while (!sync2.load(std::memory_order_acquire)) { ++counter; }
    std::cout << "(thread_three) counter = " << counter << '\n';
    
    assert(data[0].load(std::memory_order_relaxed) == 0);
    assert(data[1].load(std::memory_order_relaxed) == 1);
    assert(data[2].load(std::memory_order_relaxed) == 2);
    assert(data[3].load(std::memory_order_relaxed) == 3);
    assert(data[4].load(std::memory_order_relaxed) == 4);
}

int main()
{
    std::thread t1(thread_one);
    std::thread t2(thread_two);
    std::thread t3(thread_three);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (thread_two)   counter = 0
// (thread_three) counter = 10346
// Program ended with exit code: 0
