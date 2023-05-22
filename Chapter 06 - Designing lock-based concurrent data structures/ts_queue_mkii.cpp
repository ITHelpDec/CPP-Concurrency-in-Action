#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <iostream>

namespace ts {
template <typename T>
class queue {
public:
    queue() noexcept { }
    
    template <typename V>
    void push(V &&val)
    {
        auto sp = std::make_shared<T>(std::move(val));
        
        std::lock_guard lock(m);
        // data.push(std::forward<V>(val));
        data.push(sp);
        cv.notify_one();
    }
    
    void wait_and_pop(T &val)
    {
        std::unique_lock lock(m);
        cv.wait(lock, [this] () { return !data.empty(); } );
        // val = std::move(data.front());
        val = std::move(*data.front());
        data.pop();
    }
    
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock lock(m);
        cv.wait(lock, [this] () { return !data.empty(); } );
        // auto result = std::make_shared<T>(std::move(data.front()));
        auto result = data.front();
        data.pop();
        return result;
    }
    
    bool try_pop(T &val)
    {
        std::lock_guard lock(m);
        if (data.empty()) { return false; }
        // val = std::move(data.front());
        val = std::move(*data.front());
        data.pop();
        return true;
    }
    
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard lock(m);
        if (data.empty()) { return std::shared_ptr<T>(); }
        // auto result = std::make_shared<T>(std::move(data.front()));
        auto result = data.front();
        data.pop();
        return result;
    }
    
    bool empty() const
    {
        std::lock_guard lock(m);
        return data.empty();
    }
    
protected:
    // std::queue<T> data;
    std::queue<std::shared_ptr<T>> data;
private:
    mutable std::mutex m;
    std::condition_variable cv;
};
} // namespace ts

int main()
{
    std::cout << "Initialising ts::queue<int>...\n\n";
    ts::queue<int> q;
    // q.push(1);
    // q.push(2);
    
    int lvalue = 69;
    int pop1, pop2;
    
    std::thread t1( [&] () { q.push(42); } );
    std::thread t2( [&] () { q.push(lvalue); } );
    std::thread t3( [&] () { q.wait_and_pop(pop1); } );
    
    auto f1 = std::async(std::launch::async, [&] () { return q.wait_and_pop(); } );
    auto f2 = std::async(std::launch::async, [&] () { return q.try_pop(pop2); } );
    auto f3 = std::async(std::launch::async, [&] () { return q.try_pop(); } );
    auto f4 = std::async(std::launch::async, [&] () { return q.empty(); } );
    
    t1.join();
    t2.join();
    t3.join();
    
    auto pop1_sp = f1.get();
    bool pop2_popped = f2.get();
    auto pop2_sp = f3.get();
    bool is_empty = f4.get();
    
    std::cout << std::boolalpha;
    
    std::cout << ".:. Results .:.\n";
    std::cout << "pop1: " << pop1 << '\n';
    
    if (pop1_sp) { std::cout << "pop1_sp: " << *pop1_sp.get() << '\n'; }
    else { std::cout << "pop1_sp is a nullptr\n"; }
    
    std::cout << "pop2: " << pop2 << '\n';
    std::cout << "pop2_popped? : " << pop2_popped << '\n';
    
    if (pop2_sp) { std::cout << "pop2_sp: " << *pop2_sp.get() << '\n'; }
    else { std::cout << "pop2_sp is a nullptr\n"; }
    
    std::cout << "q.empty(): " << is_empty << '\n';
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising ts::queue<int>...
//
// .:. Results .:.
// pop1: 69
// pop1_sp: 42
// pop2: 0
// pop2_popped? : false
// pop2_sp is a nullptr
// q.empty(): true
// Program ended with exit code: 0
