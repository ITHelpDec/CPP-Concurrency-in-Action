#include <thread>
#include <iostream>

struct func {
    int &i_;
    func(int &i) : i_(i) { }
    void operator()();
};

void do_something(int &i) { ++i; }

void do_something_in_current_thread() { }

void func::operator()()
{
    for (int j = 0; j != 100000; ++j) { do_something(i_); }
}

void f() {
    int some_local_state = 0;
    std::cout << "var: " << some_local_state << '\n';
    
    func my_func(some_local_state);
    std::thread my_thread(my_func);
    
    try {
        do_something_in_current_thread();
    } catch (...) {
        my_thread.join();
        throw;
    }
    
    my_thread.join();
    
    std::cout << "var: " << some_local_state << '\n';
}

int main()
{
    f();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// var: 0
// var: 100000
// Program ended with exit code: 0
