#include <queue>
#include <iostream>

namespace cia {
template <typename T, typename Container = std::deque<T>>
class stack {
public:
    // author's version missing default constructor needed to allow example on pg. 45
    stack() noexcept : c_() { }
    
    explicit stack(const Container &c) : c_(c) { }
    explicit stack(Container&& c) : c_(std::move(c)) { }
    
    template <class Alloc> explicit stack(const Alloc &a) : c_(a) { }
    
    template <class Alloc> stack(const Container &c, const Alloc &a) : c_(c, a) { }
    template <class Alloc> stack(Container &&c, const Alloc &a)      : c_(std::move(c), a) { }
    template <class Alloc> stack(stack &&q, const Alloc &a)          : c_(std::move(q.c), a) { }
    
    bool empty() const          { return c_.empty(); }
    std::size_t size() const    { return c_.size(); }
    
    T& top()                    { return c_.back(); }
    const T& top() const        { return c_.back(); }
    
    void push(const T& t)       { c_.push_back(t); }
    void push(T &&t)            { c_.push_back(std::move(t)); }
    
    void pop()                  { c_.pop_back(); }
    
    void swap(stack &&s)
    {
        using std::swap;
        swap(c_, s.c_);
    }
    
    template <class ...Args> void emplace(Args &&...rest)
    {
        c_.emplace_back(std::forward<Args>(rest)...);
    }
protected:
    Container c_;
};
} // namespace cia

int main()
{
    cia::stack<int> istk;
    
    for (int i = 0; i != 5; ++i) { istk.push(i); }
    for (int i = 0; i != 5; ++i) { istk.push(i + 5); }
    
    cia::stack<int> istk2;
    
    std::cout << "istk.size(): "  << istk.size()  << '\n';
    std::cout << "istk2.size(): " << istk2.size() << '\n';
    
    istk2.swap(std::move(istk));
    
    std::cout << "istk.size(): "  << istk.size()  << '\n';
    std::cout << "istk2.size(): " << istk2.size() << '\n';
    
    while (!istk2.empty()) {
        std::cout << istk2.top() << ' ';
        istk2.pop();
    } std::cout << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// istk.size(): 10
// istk2.size(): 0
// istk.size(): 0
// istk2.size(): 10
// 9 8 7 6 5 4 3 2 1 0
// Program ended with exit code: 0
