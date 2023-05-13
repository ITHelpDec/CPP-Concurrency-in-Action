#include <atomic>
#include <iostream>
#include <thread>

int global_data[] = { 1, 2, 3, 4 };

std::atomic<int> idx;

void do_something_with(int i) {
    std::cout << "(do_something_with) i = " << i << '\n';
}

void e() {
    idx.store(2, std::memory_order_release);
}

void f() {
    int i = idx.load(std::memory_order_consume);
    do_something_with(global_data[std::kill_dependency(i)]);
}

int main()
{
    std::thread t1(e);
    std::thread t2(f);
    
    t1.join();
    t2.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (do_something_with) i = 3
// Program ended with exit code: 0
