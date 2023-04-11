#include <queue>
#include <iostream>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace ts {

template <typename T>
class queue {
public:
    // heavily simplified
    
    queue() = default;
    // ~queue() = default;
    
    queue(const queue&) = default;
    // queue(queue&&) = default;
    
    // "assignment deleted for simplicity" - - - - - - - - - - - - - - - - - - -
    
    queue& operator=(const queue&) = delete;
    queue& operator=(queue&&) = delete;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    // explicit queue(const Container &c) : c_(c) { }
    // explicit queue(Container &&c) noexcept : c_(std::move(c)) { }
    
    // template <class Alloc> explicit queue(const Alloc &a) : c_(a) { }
    
    // template <class Alloc> queue(const Container &c, const Alloc &a) : c_(c, a) { }
    // template <class Alloc> queue(Container &&c, const Alloc &a) noexcept
    // : c_(std::move(c), a) { }
    
    // template <class Alloc> queue(const queue &q, const Alloc &a) : c_(q.c_, a) { }
    // template <class Alloc> queue(queue &&q, const Alloc &a) noexcept
    // : c_(std::move(q.c_), a) { }
    
    // bool empty() const { return q_.empty(); }
    bool empty() const
    {
        std::lock_guard lock(m_);
        return q_.empty();
    }
    
    
    // std::size_t size() const { return c_.size(); }
    
    // T& front() { return c_.front(); }
    // const T& front() const { return c_.front(); }
    
    // T& back() { return c_.back(); }
    // const T& back() const { return c_.back(); }
    
    // void push(const T &t) { c_.push_back(t); }
    // void push(T &&t) noexcept { c_.push_back(std::move(t)); }
    
    void push(T t)
    {
        std::lock_guard lock(m_);
        
        q_.push(t);
        // q_.push(std::forward<decltype(t)>(t));
        
        cv_.notify_one();
    }
    
    // void pop() { c_.pop_front(); }
    std::shared_ptr<T> try_pop();
    bool try_pop(T &t);
    
    // std::shared_ptr<T> wait_and_pop();
    void wait_and_pop(T &t)
    {
        std::unique_lock lock(m_);
        
        std::cout << "(w&p): Waiting for the queue to populate...\n";
        cv_.wait(lock, [this] () { return !q_.empty(); } );
        
        std::cout << "(w&p): Capturing \"" << q_.front() << "\" and popping it from the queue...\n";
        t = q_.front();
        q_.pop();
    }
    
    // void swap(queue &q) noexcept
    // {
    //     using std::swap;
    //     swap(c_, q.c_);
    // }
    
    // template <class... Args> void emplace(Args &&...rest )
    // {
    //     c_.emplace_back(std::forward<Args>(rest)...);
    // }
    
private:
    std::queue<T> q_;
    
    std::mutex m_;
    std::condition_variable cv_;
};

} // namespace ts;

void sleep_then_push(int sleep_time, int val, ts::queue<int> &q) {
    std::cout << "(s&p): Sleeping for " << sleep_time << " seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(val));
    
    std::cout << "(s&p): Adding " << val << " to our queue...\n";
    q.push(val);
}

int main()
{
    std::cout << std::boolalpha;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "(main): Instantiating a threadsafe queue...\n";
    ts::queue<int> iq;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    int sleep_time = 5, val = 10;
    std::cout << "(main): Creating a thread to sleep for " << sleep_time << " seconds then push " << val << " to our queue...\n";
    std::thread t1(sleep_then_push, 5, 10, std::ref(iq));
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "(main): Attempting to pop a value with another thread...\n";
    
    int popped;
    std::thread t2(&ts::queue<int>::wait_and_pop, &iq, std::ref(popped));
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    t1.join();
    t2.join();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "(main): Value popped (" << popped << ")\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (main): Instantiating a threadsafe queue...
// (main): Creating a thread to sleep for 5 seconds then push 10 to our queue...
// (main): Attempting to pop a value with another thread...
// (s&p): Sleeping for 5 seconds...
// (w&p): Waiting for the queue to populate...
// (s&p): Adding 10 to our queue...
// (w&p): Capturing "10" and popping it from the queue...
// (main): Value popped (10)
// Program ended with exit code: 0
