#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> sync(0);

void thread_one() {
    sync.store(1, std::memory_order_release);
}

void thread_two() {
    int counter = 0;
    int expected = 1;
    
    // while (!sync.compare_exchange_strong(expected, 2, std::memory_order_acq_rel)) {
    // ...
    // why compare_exchange_strong()?
    // compare_exchange_weak() was recommended when using loops in pg. 135
    
    while (!sync.compare_exchange_weak(expected, 2, std::memory_order_acq_rel)) {
        expected = 1;
        ++counter;
    }
    
    std::cout << "(thread_two)   counter = " << counter << '\n';
}

void thread_three() {
    int counter = 0;
    while (!sync.load(std::memory_order_acquire)) { ++counter; }
    std::cout << "(thread_three) counter = " << counter << '\n';
}

int main()
{
    std::thread t1(thread_one);
    std::thread t2(thread_two);
    std::thread t3(thread_three);
    
    t1.join();
    t2.join();
    t3.join();
    
    assert(sync == 2);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (thread_two)   counter = 0
// (thread_three) counter = 0
// Program ended with exit code: 0
