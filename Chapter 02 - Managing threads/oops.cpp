#include <iostream>
#include <thread>

struct Func {
    int &i_;
    
    Func(int &i) : i_(i) { }
    
    void operator()();
    void oops();
};

void Func::operator()()
{
    for (std::size_t j = 0; j != 1000000; ++j) { std::cout << "woof!\n"; }
}

void Func::oops()
{
    int some_local_state = 0;
    Func my_func(some_local_state);
    
    std::thread my_thread(my_func);
    my_thread.detach();
}
