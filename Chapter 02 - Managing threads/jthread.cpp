#include <thread>
#include <iostream>

class jthread {
public:
    jthread() noexcept = default;
    
    template <typename Callable, typename ...Args>
    explicit jthread(Callable &&func, Args &&...rest) : t_(std::forward<Callable>(func), std::forward<Args>(rest)...) { }
    
    explicit jthread(std::thread t) noexcept : t_(std::move(t)) { }
    jthread& operator=(std::thread other) noexcept
    {
        if (joinable()) { join(); }
        t_ = std::move(other);
        return *this;
    }
    
    jthread(jthread &&other) noexcept : t_(std::move(other.t_)) { }
    jthread& operator=(jthread &&other) noexcept
    {   // this will only work if you build a joinable() member function...
        if (joinable()) { join(); }
        t_ = std::move(other.t_);
        return *this;
    }
    
    ~jthread() noexcept
    {
        if (joinable()) { join(); }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::thread::id get_id() const noexcept { return t_.get_id(); }
    
    bool joinable() const noexcept { return t_.joinable(); }
    
    void join() { t_.join(); }
    void detach() { t_.detach(); }
    void swap(jthread &other) noexcept { t_.swap(other.t_); }
    
    std::thread& as_thread() noexcept { return t_; }
    const std::thread& as_thread() const noexcept { return t_; }
    
private:
    std::thread t_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void woof() { std::cout << "woof\n"; }

void test(bool flag, int two, double pi) { std::cout << flag << ' ' << two << ' ' << pi << '\n'; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    jthread t(woof);
    
    jthread t2(test, true, 2, 3.14);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// woof
// 1 2 3.14
// Program ended with exit code: 0
