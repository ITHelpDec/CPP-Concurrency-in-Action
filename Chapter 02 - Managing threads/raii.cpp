#include <thread>
#include <iostream>

class thread_guard {
public:
    explicit thread_guard(std::thread &t) : t_(t) { }
    
    ~thread_guard()
    {
        if (t_.joinable()) { t_.join(); }
    }
    
    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
private:
    std::thread &t_;
};

struct func {
    int &i_;
    func(int &i) : i_(i) { }
    void operator()();
};

void do_something(int &i) { ++i; }

void do_something_in_current_thread() { std::cout << "woof!\n"; }

void func::operator()()
{
    for (int j = 0; j != 100000; ++j) { do_something(i_); }
}

void f() {
    int some_local_state = 0;
    func my_func(some_local_state);
    std::thread t(my_func);
    
    thread_guard g(t);
    
    do_something_in_current_thread();
}

int main()
{
    f();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// woof!
// Program ended with exit code: 0
