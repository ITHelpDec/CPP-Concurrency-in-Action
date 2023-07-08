#include <atomic>
#include <memory>
#include <functional>
#include <vector>
#include <thread>
#include <numeric>
#include <iostream>
#include <future>

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
        
        {
            std::lock_guard lock(tail_m);
            tail_->data_ = new_data;
            node *new_tail = p.get();
            tail_->next_ = std::move(p);
            tail_ = new_tail;
        }
        
        cv.notify_one();
    }
    
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
        std::unique_ptr<node> old_head = std::move(head_);
        head_ = std::move(old_head->next_);
        return old_head;
    }
    
    std::unique_lock<std::mutex> wait_for_data()
    {
        std::unique_lock<std::mutex> lock(head_m);
        cv.wait(lock, [&] () { return head_.get() != get_tail(); } );
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
    
    mutable std::mutex head_m;
    mutable std::mutex tail_m;
    
    std::condition_variable cv;
};
} // namespace mt (multi-threaded)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class join_threads {
public:
    explicit join_threads(std::vector<std::thread> &threads) : threads_(threads) { }
    
    ~join_threads()
    {
        for (auto &t : threads_)
            if (t.joinable()) { t.join(); }
    }
    
private:
    std::vector<std::thread> &threads_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class function_wrapper {
public:
    function_wrapper() = default;
    
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
    
    function_wrapper(function_wrapper &&other) noexcept : impl_(std::move(other.impl_)) { }
    
    function_wrapper& operator=(function_wrapper &&rhs) noexcept
    {
        impl_ = std::move(rhs.impl_);
        return *this;
    }
    
    template <typename Func>
    // function_wrapper(Func &&f) : impl_(new impl_type<Func>(std::move(f))) { }
    function_wrapper(Func &&f) noexcept : impl_(std::make_unique<impl_type<Func>>(std::move(f))) { }
    
    void operator() () { impl_->call(); }
    
    
private:
    struct impl_base {
        // abstract base class
        virtual void call() = 0;
        virtual ~impl_base() { }
    };
    
    std::unique_ptr<impl_base> impl_;
    
    template <typename Func>
    struct impl_type : impl_base {
        Func f_;
        
        impl_type(Func &&f) : f_(std::move(f)) { }
        void call() { f_(); }
    };
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class thread_pool {
public:
    thread_pool() : done_(false), joiner_(threads_)
    {
        std::size_t thread_count = std::thread::hardware_concurrency();
        
        try {
            for (int i = 0; i != thread_count; ++i)
                threads_.push_back(std::thread(&thread_pool::worker_thread, this));
        } catch (...) {
            done_ = true;
            throw;
        }
    }
    
    ~thread_pool() { done_ = true; }
    
    // previous `.submit()` function
    // template <typename Func>
    // void submit(Func f)
    // {
    //     workq_.push(std::function<void()>(f));
    // }
    
    template <typename Func>
    std::future<std::invoke_result_t<Func&&>> submit(Func f) // std::result_of is deprecated
    {
        typedef std::invoke_result_t<Func&&> T;
        
        std::packaged_task<T()> task(std::move(f));
        std::future<T> result(task.get_future());
        workq_.push(std::move(task));
        
        return result;
    }
    
private:
    // std::atomic_bool done;
    // https://comp.std.cpp.narkive.com/nwMakWji/std-atomic-bool-vs-std-atomic-bool
    std::atomic<bool> done_;
    
    mt::queue<function_wrapper> workq_;
    
    std::vector<std::thread> threads_;
    join_threads joiner_;
    
    void worker_thread()
    {
        while (!done_) {
            function_wrapper task;
            
            if (workq_.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
            
        }
    }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void go_forth_and_multiply(std::vector<int> &ivec) {
    thread_pool tp;
    for (int &i : ivec) { tp.submit([&] () { i *= i; }); }
}

void printVec(const std::vector<int> &ivec) {
    for (int i : ivec) {
        std::cout << i << ' ';
    } std::cout << '\n';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::vector<int> ivec(10);
    std::iota(ivec.begin(), ivec.end(), 1);
    
    std::cout << "ivec: "; printVec(ivec);
    
    go_forth_and_multiply(ivec);
    
    std::cout << "ivec: "; printVec(ivec);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ivec: 1 2 3 4 5 6 7 8 9 10
// ivec: 1 4 9 16 25 36 49 64 81 100
// Program ended with exit code: 0
