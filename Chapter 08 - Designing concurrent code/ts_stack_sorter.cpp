#include <stack>
// #include <mutex>
// #include <memory>
#include <iostream>
#include <list>
#include <future>
// #include <vector>
// #include <thread>
// #include <atomic>

namespace ts {
template <typename T>
class stack {
public:
    stack() { }
    
    stack(const stack &o)
    {
        std::lock_guard lock(m_);
        data_ = o.data_;
    }
    
    stack& operator=(const stack&) = delete;
    
    template <typename V>
    void push(V &&val)
    {
        std::lock_guard lock(m_);
        data_.push(std::forward<V>(val));
    }
    
    std::shared_ptr<T> pop()
    {
        std::lock_guard lock(m_);
        if (data_.empty()) { return std::shared_ptr<T>(); }
        
        // EXTREMELY IMPORTANT TO USE STD::MOVE
        // "No matching constructor for 'construct_at'", otherwise
        auto result = std::make_shared<T>(std::move(data_.top()));
        data_.pop();
        return result;
    }
    
    bool empty() const
    {
        std::lock_guard lock(m_);
        return data_.empty();
    }
private:
    std::stack<T> data_;
    mutable std::mutex m_;
};
} // namespace ts (threadsafe)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
class sorter {
public:
    sorter()
    {
        max_threads_ = std::thread::hardware_concurrency() - 1;
        still_data_.store(false);// = false;
    }
    
    ~sorter()
    {
        still_data_.store(true);// = true;
        for (auto &t : threads_) { t.join(); }
    }
    
    std::list<T> do_sort(std::list<T> &chunk_data)
    {
        if (chunk_data.empty()) { return chunk_data; }
        
        std::list<T> result;
        
        // like in chapter 4 - splice(a, b, c) -> transfer c from b before a
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        
        const T &partition_val = *(result.begin());
        
        // opted for "auto"
        // split em up; less than pivot to the left; more than pivot to the right
        auto divide_point = std::partition(chunk_data.begin(), chunk_data.end(), [&] (const T &val) {
            return val < partition_val;
        });
        
        chunk_to_sort new_lower_chunk;
        
        // splice(a, b, c, d) -> transfer range (c, d] from b to before a
        new_lower_chunk.data_.splice(new_lower_chunk.data_.end(), chunk_data, chunk_data.begin(), divide_point);
        
        std::future<std::list<T>> new_lower = new_lower_chunk.promise_.get_future();
        
        chunks_.push(std::move(new_lower_chunk));
        
        if (threads_.size() < max_threads_) { threads_.push_back(std::thread([this] () { sort_thread(); } )); }
        
        std::list<T> new_higher(do_sort(chunk_data));
        
        // splice (a, b) -> transfer all of b just before a
        result.splice(result.end(), new_higher);
        
        while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            try_sort_chunk();
        }
        
        // splice (a, b) -> transfer all of b just before a
        result.splice(result.begin(), new_lower.get());
        
        return result;
    }
private:
    struct chunk_to_sort {
        std::list<T> data_;
        std::promise<std::list<T>> promise_;
    };
    
    ts::stack<chunk_to_sort> chunks_;
    
    std::vector<std::thread> threads_;
    
    std::size_t max_threads_;
    
    std::atomic<bool> still_data_;
    
    void try_sort_chunk()
    {
        auto chunk = chunks_.pop();
        if (chunk) { sort_chunk(chunk); }
    }
    
    void sort_chunk(const std::shared_ptr<chunk_to_sort> &chunk)
    {
        chunk->promise_.set_value(do_sort(chunk->data_));
    }
    
    void sort_thread()
    {
        while (!still_data_) {
            try_sort_chunk();
            std::this_thread::yield();
            
            // yield allows the scheduler to "give way" to other threads
            // "this should be used in a case where you are in a busy waiting state, like in a thread pool:"
            // https://stackoverflow.com/a/11049210
        }
    }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace par {
template <typename T>
std::list<T> quick_sort(std::list<T> input) {
    if (input.empty()) { return input; }
    
    return sorter<T>().do_sort(input);
}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
void print_list(const std::list<T> &tl) {
    for (auto &&t : tl) {
        std::cout << t << ' ';
    } std::cout << '\n';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::list<int> il = { 1, 5 ,4, 2, 3, 6, 7, 2, 1, 5 ,4, 3, 6, 7 };
    std::cout << "before: ";
    print_list(il);
    
    auto sorted = par::quick_sort(il);
    std::cout << "after:  ";
    print_list(sorted);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// before: 1 5 4 2 3 6 7 2 1 5 4 3 6 7
// after:  1 1 2 2 3 3 4 4 5 5 6 6 7 7
// Program ended with exit code: 0
