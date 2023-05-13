#include <thread>
#include <iostream>

struct X {
    int i;
    std::string s;
};

std::atomic<X*> ap;
std::atomic<int> ai;

void create_x() {
    X *x = new X;
    x->i = 42;
    x->s = "hello";
    
    ai.store(99, std::memory_order_relaxed);
    ap.store(x, std::memory_order_release);
}

void use_x() {
    X *x;
    
    int counter = 0;
    while (! (x = ap.load(std::memory_order_consume)) ) {
        ++counter;
        // std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    
    std::cout << "(use_x) counter = " << counter << '\n';
    
    assert(x->i == 42);
    assert(x->s == "hello");
    assert(ai == 99);
    
    // delete x;
}

int main()
{
    std::thread t1(create_x);
    std::thread t2(use_x);
    
    t1.join();
    t2.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (use_x) counter = 291
// Program ended with exit code: 0
