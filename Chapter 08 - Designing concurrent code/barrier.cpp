#include <iostream>
#include <atomic>
#include <thread>

class barrier {
public:
    explicit barrier(std::size_t count) : count_(count), generation_(0)
    {
        spaces_ = count_.load();
    }
    
    void wait()
    {
        std::size_t my_gen = generation_;
        
        if (!--spaces_) {
            spaces_ = count_.load();
            ++generation_;
        } else {
            while (generation_ == my_gen) { std::this_thread::yield(); }
        }
    }
    
    void done_waiting()
    {
        --count_;
        
        if (!--spaces_) {
            spaces_ = count_.load();
            ++generation_;
        }
    }
    
private:
    std::atomic<std::size_t> count_;
    
    std::atomic<std::size_t> spaces_;
    std::atomic<std::size_t> generation_;
};
