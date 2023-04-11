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
    queue() = default;
    queue(const queue &other)
    {
        // no need to lock current mutex because it's still under construction
        std::lock_guard lock(other.m_);
        q_ = other.q_;
    }
    
    // "assignment deleted for simplicity" - - - - - - - - - - - - - - - - - - -
    
    queue& operator=(const queue&) = delete;
    queue& operator=(queue&&) = delete;
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void push(T val)
    {
        std::lock_guard lock(m_);
        q_.push(val);
        cv_.notify_one();
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    void wait_and_pop(T &t)
    {
        std::unique_lock lock(m_);
        
        std::cout << "(w&p): Waiting for the queue to populate...\n";
        cv_.wait(lock, [this] () { return !q_.empty(); } );
        
        std::cout << "(w&p): Capturing \"" << q_.front() << "\" and popping it from the queue...\n";
        t = q_.front();
        q_.pop();
    }
    
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock lock(m_);
        cv_.wait(lock, [this] () { return !q_.empty(); } );
        
        // std::shared_ptr<T> res(std::make_shared<T>(q_.front()));
        auto result(std::make_shared<T>(q_.front()));
        q_.pop();
        
        return result;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    bool try_pop(T &poppee)
    {
        std::lock_guard lock(m_);
        
        if (q_.empty()) { return false; }
        
        poppee = q_.front();
        q_.pop();
        
        return true;
    }
    
    std::shared_ptr<T> try_pop()
    {
        std::lock_guard lock(m_);
        
        if (q_.empty()) { return std::shared_ptr<T>(); }
        
        auto result(std::make_shared<T>(q_.front()));
        q_.pop();
        
        return result;
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    bool empty() const
    {
        std::lock_guard lock(m_);
        return q_.empty();
    }
    
private:
    std::queue<T> q_;
    
    std::mutex m_;
    std::condition_variable cv_;
};

} // namespace ts;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void sleep_then_push(int sleep_time, int val, ts::queue<int> &q) {
    std::cout << "(s&p): Sleeping for " << sleep_time << " seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(val));
    
    std::cout << "(s&p): Adding " << val << " to our queue...\n";
    q.push(val);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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
    
    std::cout << "(main): Starting another thread to wait until the queue is no longer empty...\n";
    
    int popped;
    
    // lambda notation because of member function overload ambiguity
    std::thread t2( [&popped, &iq] () { return iq.wait_and_pop(popped); } );
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "(main): Attempting to pop a value from the queue...\n";
    
    bool result;
    std::thread t3( [&iq, &popped, &result] () { return result = iq.try_pop(popped); } );
    std::cout << "(main): pop worked?\n" << result << " (popped = " << popped << ")\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "(main): Attempting to pop a value a return a shared_ptr...\n";
    
    std::shared_ptr<int> sp;
    std::thread t4( [&iq, &sp] () { return sp = iq.try_pop(); } );
    std::cout << "shared_ptr: " << sp.get() << " (nullptr)\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
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
// (main): Starting another thread to wait until the queue is no longer empty...
// (main): Attempting to pop a value from the queue...
// (s&p): Sleeping for 5 seconds...
// (main): pop worked?
// (w&p): Waiting for the queue to populate...
// false (popped = 0)
// (main): Attempting to pop a value a return a shared_ptr...
// shared_ptr: 0x0 (nullptr)
// (s&p): Adding 10 to our queue...
// (w&p): Capturing "10" and popping it from the queue...
// (main): Value popped (10)
// Program ended with exit code: 0
