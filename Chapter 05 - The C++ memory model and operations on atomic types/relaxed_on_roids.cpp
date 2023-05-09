#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> x = 0, y = 0, z = 0;
std::atomic<bool> go = false;

const std::size_t loop_count = 10;

struct read_values {
    int x_, y_, z_;
};

read_values rv1[loop_count];
read_values rv2[loop_count];
read_values rv3[loop_count];
read_values rv4[loop_count];
read_values rv5[loop_count];

void increment(std::atomic<int> *var_to_inc, read_values *rv) {
    while (!go) {
        // spin and wait for the signal
        std::this_thread::yield();
    }
    
    for (std::size_t i = 0; i != loop_count; ++i) {
        rv[i].x_ = x.load(std::memory_order_relaxed);
        rv[i].y_ = y.load(std::memory_order_relaxed);
        rv[i].z_ = z.load(std::memory_order_relaxed);
        
        var_to_inc->store(i + 1, std::memory_order_relaxed);
        
        std::this_thread::yield();
        // https://en.cppreference.com/w/cpp/thread/yield
        // "Provides a hint to the implementation to reschedule the execution of threads, ...
        // ...allowing other threads to run."

    }
}

void read(read_values *rv) {
    while (!go) {
        // spin and wait for the signal
        std::this_thread::yield();
    }
    
    for (std::size_t i = 0; i != loop_count; ++i) {
        rv[i].x_ = x.load(std::memory_order_relaxed);
        rv[i].y_ = y.load(std::memory_order_relaxed);
        rv[i].z_ = z.load(std::memory_order_relaxed);
                
        std::this_thread::yield();
        // https://en.cppreference.com/w/cpp/thread/yield
        // "Provides a hint to the implementation to reschedule the execution of threads, ...
        // ...allowing other threads to run."
    }
}

void print(read_values *v) {
    for (std::size_t i = 0; i != loop_count; ++i) {
        if (i) { std::cout << ", "; }
        std::cout << "(" << v[i].x_ << ", " << v[i].y_ << ", " << v[i].z_ << ")";
    } std::cout << '\n' << std::endl;
}

int main()
{
    std::thread t1(increment, &x, rv1);
    std::thread t2(increment, &y, rv2);
    std::thread t3(increment, &z, rv3);
    std::thread t4(read, rv4);
    std::thread t5(read, rv5);
    
    go = true;
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    
    print(rv1);
    print(rv2);
    print(rv3);
    print(rv4);
    print(rv5);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (0, 0, 0), (1, 1, 0), (2, 2, 0), (3, 3, 0), (4, 4, 0),
// (5, 5, 1), (6, 6, 1), (7, 7, 2), (8, 7, 2), (9, 7, 2)

// (1, 0, 0), (2, 1, 0), ( 3, 2, 0), ( 4, 3, 0), ( 5, 4, 0),
// (6, 5, 1), (7, 6, 2), (10, 7, 4), (10, 8, 4), (10, 9, 5)

// ( 5,  5, 0), ( 7,  6, 1), ( 9,  7, 2), (10,  7, 3), (10,  9, 4),
// (10, 10, 5), (10, 10, 6), (10, 10, 7), (10, 10, 8), (10, 10, 9)

// (0, 0, 0), (1, 1, 0), (2, 2, 0), ( 3, 3, 0), ( 4, 4, 0),
// (5, 5, 1), (6, 6, 1), (7, 7, 2), (10, 7, 3), (10, 9, 5)

// (10, 10, 10), (10, 10, 10), (10, 10, 10), (10, 10, 10), (10, 10, 10),
// (10, 10, 10), (10, 10, 10), (10, 10, 10), (10, 10, 10), (10, 10, 10)

// Program ended with exit code: 0
