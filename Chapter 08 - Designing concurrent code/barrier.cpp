#include <iostream>
#include <atomic>
#include <thread>

class barrier {
public:
    explicit barrier(std::size_t count) : count_(count), spaces_(count_), generation_(0) { }
    
    void wait()
    {
        std::size_t my_gen = generation_;
        
        if (!--spaces_) {
            spaces_ = count_;
            ++generation_;
        } else {
            while (generation_ == my_gen) { std::this_thread::yield(); }
        }
    }
    
private:
    std::size_t count_;
    
    std::atomic<std::size_t> spaces_;
    std::atomic<std::size_t> generation_;
};
