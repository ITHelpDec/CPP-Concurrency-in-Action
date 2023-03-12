#include <queue>
#include <iostream>

template <typename T, typename Container = std::deque<T>>
class stack {
public:
    explicit stack(const Container&);
    explicit stack(Container&& = Container());
    
    template <class Alloc> explicit stack(const Alloc&);
    template <class Alloc> stack(const Container&, const Alloc&);
    template <class Alloc> stack(Container&&, const Alloc&);
    template <class Alloc> stack(stack&&, const Alloc&);
    
    bool empty() const;
    std::size_t size() const;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    T& top();
    const T& top() const;
    
    // these two functions work well sequentially, but are not thread-safe
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    void push(const T&);
    void push(T&&);
    
    void pop();
    void swap(stack&&);
    
    template <class ...Args> void emplace_back(Args &&...rest);
};
