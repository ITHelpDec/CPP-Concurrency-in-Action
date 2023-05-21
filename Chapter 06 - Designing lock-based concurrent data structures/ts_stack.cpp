#include <exception>
#include <iostream>
#include <stack>
#include <memory>
#include <thread>
#include <future>

struct empty_stack : std::exception {
    const char *what() const throw() { return "woof!\n"; }
};

namespace ts {

template <typename T>
class stack {
public:
    stack() { }
    
    stack(const stack &other)
    {
        std::lock_guard lock(m);
        data = other.data;
    }
    
    stack& operator=(const stack &other) = delete;
    
    // void push(const T &new_value)
    // {
    //     std::lock_guard lock(m);
    //     data.push(new_value);
    // }
    //
    // void push(T &&new_value)
    // {
    //     std::lock_guard lock(m);
    //     data.push(std::move(new_value));
    // }
    
    template <typename U>
    void push(U &&new_value)
    {
        std::lock_guard lock(m);
        data.push(std::forward<U>(new_value));
    }
    
    std::shared_ptr<T> pop()
    {
        std::lock_guard lock(m);
        if (data.empty()) { throw empty_stack(); }
        
        // std::shared_ptr<T> const res(std::make_shared<T>(std::move(data.top())));
        // apparently, not a good idea to return a const variables
        // https://quuxplusone.github.io/blog/2022/01/23/dont-const-all-the-things/
        
        auto result = std::make_shared<T>(std::move(data.top()));
        data.pop();
        return result;
    }
    
    void pop(T &value)
    {
        std::lock_guard lock(m);
        if(data.empty()) { throw empty_stack(); }
        value = std::move(data.top());
        data.pop();
    }
    
    bool empty() const
    {
        std::lock_guard lock(m);
        return data.empty();
    }
    
protected:
    std::stack<T> data;
private:
    mutable std::mutex m;
    // "Since locking a mutex is a mutating operation, ...
    // ...the mutex object must be marked mutable, ...
    // ...so it can be locked in empty() and in the copy constructor."
};
} // namespace ts

int main()
{
    std::cout << std::boolalpha;
    
    std::cout << "initialising threadsafe stack...\n";
    ts::stack<int> istk, istk2;
    
    // auxiliary variables
    int lvalue = 69;
    int poppee;
        
    // lambdas make overloads so much easier
    // std::thread t1([&] () { istack1.push(42); });
    
    std::thread t1(static_cast<void(ts::stack<int>::*)(int&&)>(&ts::stack<int>::push), &istk, 42);
    std::thread t2(static_cast<void(ts::stack<int>::*)(int&&)>(&ts::stack<int>::push), &istk, lvalue);
    std::thread t3([&] () { ts::stack<int> istk2(istk); } ); // not quite what i wanted
    std::thread t4(static_cast<void(ts::stack<int>::*)(int&)>(&ts::stack<int>::pop), &istk, std::ref(poppee));
    
    auto f = std::async(std::launch::async, [&istk] () { return istk.pop(); } );
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    auto sp = f.get();
    
    std::cout << "istk2.empty(): " << istk2.empty() << '\n';
    std::cout << "poppee:        " << poppee        << '\n';
    std::cout << "shared_ptr:    " << *sp.get()     << '\n';
    std::cout << "istk.empty():  " << istk.empty()  << '\n';
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// initialising threadsafe stack...
// istk2.empty(): true
// poppee:        69
// shared_ptr:    42
// istk.empty():  true
// Program ended with exit code: 0
