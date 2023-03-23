#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <iostream>
#include <thread>

namespace ts {

struct empty_stack : std::exception {
    // author left what() undefined...
    // const char* what() const throw();
    
    const char* what() const throw() { return "woof, woof! exception thrown!\n"; }
};

template <typename T>
class stack {
public:
    stack() { }
    stack(const stack &other)
    {
        std::lock_guard<std::mutex> lock (other.m);
        data = other.data;
    }
    
    stack& operator=(const stack&) = delete;
    
    void push(T val)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(val));
    }
    
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) { throw empty_stack(); }
        const std::shared_ptr<T> res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    
    void pop(T& val)
    {
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) { throw empty_stack(); }
        val = data.top();
        data.pop();
    }
    
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
    
private:
    std::stack<T> data;
    mutable std::mutex m;
};

} // namespace ts

void push_to_stack(ts::stack<int> &ts_stk) {
    for (int i = 0; i != 100; ++i ) { ts_stk.push(i); }
}

void pop_from_stack(ts::stack<int> &ts_stk) {
    int top;
    for (int i = 0; i != 100; ++i ) {
        if (i % 10 == 0) {
            ts_stk.pop(top);
            std::cout << "pop_from_stack: " << top << '\n';
        } else { ts_stk.pop(); }
    }
}

int main()
{
    ts::stack<int> ts_stk;
    
    std::thread t1(push_to_stack, std::ref(ts_stk));
    std::thread t2(pop_from_stack, std::ref(ts_stk));
    
    t1.join();
    t2.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// pop_from_stack: 99
// pop_from_stack: 89
// pop_from_stack: 79
// pop_from_stack: 69
// pop_from_stack: 59
// pop_from_stack: 49
// pop_from_stack: 39
// pop_from_stack: 29
// pop_from_stack: 19
// pop_from_stack: 9
// Program ended with exit code: 0
