#include <memory>
#include <queue>
#include <iostream>

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

namespace ws {
template <typename T>
class queue {
public:
    queue() { }
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    void push(T data) {
        std::lock_guard<std::mutex> lock(m_);
        q_.push_front(std::move(data));
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_);
        return q_.empty();
    }
    
    bool try_pop(T &result) {
        std::lock_guard<std::mutex> lock(m_);
        
        if (q_.empty()) { return false; }
        
        result = std::move(q_.front());
        q_.pop_front();
        return true;
    }
    
    bool try_steal(T &result) {
        std::lock_guard<std::mutex> lock(m_);
        
        if (q_.empty()) {return false; }
        
        result = std::move(q_.back());
        q_.pop_back();
        return true;
    }
private:
    std::deque<T> q_;
    mutable std::mutex m_;
};
} // namespace ws (work-stealing)
