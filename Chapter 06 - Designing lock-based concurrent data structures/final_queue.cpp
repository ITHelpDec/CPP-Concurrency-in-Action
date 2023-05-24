#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <future>
#include <random>
#include <iostream>
#include <condition_variable>

int num() {
    static std::default_random_engine e;
    static std::uniform_int_distribution u(2, 5);
    return u(e);
}

namespace mt {
template <typename T>
class queue {
public:
    queue() : head_(std::make_unique<node>()), tail_(head_.get()) { }
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    std::shared_ptr<T> try_pop()
    {
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data_ : std::shared_ptr<T>();
    }
    
    bool try_pop(T &val)
    {
        std::unique_ptr<node> old_head = try_pop_head(val);
        // No viable conversion from returned value of type 'std::unique_ptr<node>' to function return type 'bool'
        // return old_head;
        return old_head.get();
    }
    
    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_ptr<node> old_head = wait_pop_head();
        return old_head->data_;
    }
    
    void wait_and_pop(T &val)
    {
        std::unique_ptr<node> old_head = wait_pop_head(val);
    }
    
    template <typename V>
    void push(V &&val)
    {
        auto new_data = std::make_shared<T>(std::forward<V>(val));
        auto p = std::make_unique<node>();
        
        // new_tail moved inside lock
        // node *new_tail = p.get();
        
        {
            std::lock_guard lock(tail_m);
            tail_->data_ = new_data;
            node *new_tail = p.get();
            tail_->next_ = std::move(p);
            tail_ = new_tail;
        }
        
        // notify outside of lock
        cv.notify_one();
    }
    
    // make const
    // bool empty()
    bool empty() const
    {
        std::lock_guard lock(head_m);
        return head_.get() == get_tail();
    }
    
private:
    struct node
    {
        std::shared_ptr<T> data_;
        std::unique_ptr<node> next_;
    };
    
    std::unique_ptr<node> pop_head()
    {
        // lock and comparison removed
        // std::lock_guard lock(head_m);
        // if (head_.get() == get_tail()) { return nullptr; }
        
        std::unique_ptr<node> old_head = std::move(head_);
        head_ = std::move(old_head->next_);
        return old_head;
    }
    
    // pair cv's with unique locks
    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> lock(head_m);
        cv.wait(lock, [&] () { return head_.get() != get_tail(); } );
        
        // Moving a local object in a return statement prevents copy elision
        // return std::move(lock);
        return lock;
    }
    
    std::unique_ptr<node> wait_pop_head()
    {
        std::unique_lock<std::mutex> lock(wait_for_data());
        return pop_head();
    }
    
    std::unique_ptr<node> wait_pop_head(T &val)
    {
        std::unique_lock<std::mutex> lock(wait_for_data());
        val = std::move(*head_->data_);
        return pop_head();;
    }
    
    // make const
    // node* get_tail()
    node* get_tail() const
    {
        std::lock_guard lock(tail_m);
        return tail_;
    }
    
    std::unique_ptr<node> try_pop_head()
    {
        std::lock_guard lock(head_m);
        if (head_.get() == get_tail()) { return std::unique_ptr<node>(); }
        return pop_head();
    }
    
    std::unique_ptr<node> try_pop_head(T &val)
    {
        std::lock_guard lock(head_m);
        if (head_.get() == get_tail()) { return std::unique_ptr<node>(); }
        val = std::move(*head_->data_);
        return pop_head();
    }
    
    std::unique_ptr<node> head_;
    node *tail_;
    
    // make mutexes mutable to work on empty() const
    mutable std::mutex head_m;
    mutable std::mutex tail_m;
    
    // addition of condition_variable
    std::condition_variable cv;
};
} // namespace mt (multi-threaded)

int main()
{
    std::cout << "Initialising queue...\n";
    mt::queue<int> q;
    
    int x;
    std::cout << "Launching waiter thread...\n";
    std::thread t1( [&] () { q.wait_and_pop(x); } );
    
    std::cout << "Is our queue empty? " << (q.empty() ? "Yes\n\n" : "No\n\n");
    
    std::cout << "Launching a separate thread to try pop after two seconds...\n";
    auto f1 = std::async(std::launch::async, [&] () {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return q.try_pop();
    });
    
    std::cout << "Was our std::shared_ptr pop successful?...";
    auto sp1 = f1.get();
    std::cout << (sp1 ? "yes " : "no ") << "(" << sp1.get() << ")\n\n";
    
    std::cout << "Main thread sleeping for 4 seconds before pushing...\n";
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    std::cout << "Pushing an rvalue (69)...\n";
    q.push(69);
    
    t1.join();
    
    std::cout << x << " was popped from the front of the queue.\n";
    
    std::cout << "Is our queue empty again? " << (q.empty() ? "Yes\n\n" : "No\n\n");
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "Launching another waiter...\n";
    auto f2 = std::async(std::launch::async, [&] () { return q.wait_and_pop(); } );
    
    int y = -1;
    std::cout << "Launching yet another thread to try pop after two seconds...\n";
    auto f3 = std::async(std::launch::async, [&] () {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return q.try_pop(y);
    });
    
    std::cout << "Was our bool pop successful?...";
    bool pop = f3.get();
    std::cout << (pop ? "yes " : "no ") << "(returns " << y << ")\n\n";
    
    std::cout << "Main thread sleeping for another 4 seconds before pushing...\n";
    std::this_thread::sleep_for(std::chrono::seconds(4));
    
    int lvalue = 42;
    std::cout << "Pushing an lvalue (" << lvalue << ")...\n";
    q.push(lvalue);
    
    auto sp2 = f2.get();
    std::cout << *sp2.get() << " was popped from the front of the queue this time.\n";
    
    std::cout << "Is our queue empty for the last time? " << (q.empty() ? "Yes\n\n" : "No\n\n");
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising queue...
// Launch waiter thread...
// Is our queue empty? Yes

// Launching a separate thread to try pop after two seconds...
// Was our std::shared_ptr pop successful?...no (0x0)

// Main thread sleeping for 4 seconds before pushing...
// Pushing an rvalue (69)...
// 69 was popped from the front of the queue.
// Is our queue empty again? Yes

// Launching another waiter...
// Launching yet another thread to try pop after two seconds...
// Was our bool pop successful?...no (returns -1)

// Main thread sleeping for another 4 seconds before pushing...
// Pushing an lvalue (42)...
// 42 was popped from the front of the queue this time.
// Is our queue empty for the last time? Yes

// Program ended with exit code: 0
