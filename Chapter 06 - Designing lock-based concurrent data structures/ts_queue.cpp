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
        std::lock_guard lock(m);
        data.push(std::forward<V>(val));
        cv.notify_one();
    }
    
    void wait_and_pop(T &val)
    {
        std::unique_lock lock(m);
        cv.wait(lock, [this] () { return !data.empty(); } );
        val = std::move(data.front());
        data.pop();
    }
    
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock lock(m);
        cv.wait(lock, [this] () { return !data.empty(); } );
        auto result = std::make_shared<T>(std::move(data.front()));
        data.pop();
        return result;
    }
    
    bool try_pop(T &val)
    {
        std::lock_guard lock(m);
        if (data.empty()) { return false; }
        val = std::move(data.front());
        data.pop();
        return true;
    }
    
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard lock(m);
        if (data.empty()) { return std::shared_ptr<T>(); }
        auto result = std::make_shared<T>(std::move(data.front()));
        data.pop();
        return result;
    }
    
    bool empty() const
    {
        std::lock_guard lock(m);
        return data.empty();
    }
    
protected:
    std::queue<T> data;
private:
    mutable std::mutex m;
    std::condition_variable cv;
};
} // namespace ts

int main()
{
    std::cout << "Initialising ts::queue<int>...\n";
    ts::queue<int> q;
    q.push(1);
    q.push(2);
    
    bool empty = false;
    int lvalue = 69;
    int poppee_1, poppee_2;
    bool poppee_2_worked = false;
    
    std::cout << "Launching threads and futures...\n\n";
    std::thread t1( [&] () { q.wait_and_pop(poppee_1); } );
    auto f1 = std::async(std::launch::async, [&] () { return q.wait_and_pop(); } );
    std::thread t2( [&] () { q.push(42); } );
    std::thread t3( [&] () { q.push(lvalue); } );
    auto f2 = std::async(std::launch::async, [&] () { return q.try_pop(poppee_2); } );
    auto f3 = std::async(std::launch::async, [&] () { return q.try_pop(); } );
    auto f4 = std::async(std::launch::async, [&] () { return q.empty(); } );
    
    t1.join();
    auto smart_poppee_1 = f1.get();
    t2.join();
    t3.join();
    poppee_2_worked = f2.get();
    auto smart_poppee_2 = f3.get();
    empty = f4.get();
    
    std::cout << std::boolalpha;
    
    std::cout << ".:. Results .:.\n";
    std::cout << "empty(): " << empty << '\n';
    std::cout << "lvalue: " << lvalue << '\n';
    std::cout << "poppee_1: " << poppee_1 << '\n';
    std::cout << "poppee_2: " << poppee_2 << '\n';
    std::cout << "poppee_2_worked: " << poppee_2_worked << '\n';
    std::cout << "smart_poppee_1: " << *smart_poppee_1.get() << '\n';
    std::cout << "smart_poppee_2: " << *smart_poppee_2.get() << '\n';
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising ts::queue<int>...
// Launching threads and futures...

// .:. Results .:.
// empty(): true
// lvalue: 69
// poppee_1: 1
// poppee_2: 42
// poppee_2_worked: true
// smart_poppee_1: 2
// smart_poppee_2: 69
// Program ended with exit code: 0
