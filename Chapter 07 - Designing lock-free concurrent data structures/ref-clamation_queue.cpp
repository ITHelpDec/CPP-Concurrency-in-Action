#include <iostream>
#include <future>
#include <random>

namespace lf {
template <typename T>
class queue {
public:
    queue() : head_(std::make_shared<node>()), tail_(std::atomic_load(&head_)) { }
    
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    ~queue()
    {
        while (auto old_head = std::atomic_load(&head_)) {
            std::atomic_store(&head_, old_head->next_);
            old_head.reset();
        }
    }
    
    // ~queue() { while(pop_head()); }
    
    template <typename V>
    void push(V &&val)
    {
        auto new_data = std::make_shared<T>(std::forward<V>(val));
        auto new_next = std::make_shared<node>();
        
        auto old_tail = std::atomic_load(&tail_);
        
        old_tail->data_.swap(new_data);
        old_tail->next_ = new_next;
        
        std::atomic_store(&tail_, new_next);
    }
    
    std::shared_ptr<T> pop()
    {
        auto old_head = pop_head();
        
        if (!old_head) { return std::shared_ptr<T>(); }
        
        auto result = old_head->data_;
        old_head.reset();
        return result;
    }
    
    std::shared_ptr<T> front()
    {
        auto current = std::atomic_load(&head_);
        return current->data_;
    }
private:
    struct node {
        std::shared_ptr<T> data_;
        std::shared_ptr<node> next_;
        
        node() = default;
    };
    
    std::shared_ptr<node> head_;
    std::shared_ptr<node> tail_;
    
    std::shared_ptr<node> pop_head()
    {
        auto old_head = std::atomic_load(&head_);
        
        if (old_head == std::atomic_load(&tail_)) { return std::shared_ptr<node>(); }
        
        while (old_head && !std::atomic_compare_exchange_weak(&head_, &old_head, old_head->next_));
        
        return old_head;
    }
};
} // namespace lf (lock-free)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

constexpr std::size_t num_threads = 10000;

template <typename T>
void fill(lf::queue<T> &q) {
    std::vector<std::future<void>> threads(num_threads);
    
    for (int i = 0; i != num_threads; ++i) {
        threads[i] = std::async(std::launch::async, [&, i] () { q.push(i); });
    }
    
    for (auto &f : threads) { f.get(); }
}

template <typename T>
std::shared_ptr<int> clear(lf::queue<T> &q) {
    std::vector<std::future<std::shared_ptr<T>>> threads(num_threads);
    
    for (auto &f : threads) {
        f = std::async(std::launch::async, [&] () { return q.pop(); });
    }
    
    std::shared_ptr<T> result, temp;
    for (auto &f : threads) {
        temp = f.get();
        if (temp) { result = temp; }
    }
    
    return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    lf::queue<int> q;
    
    std::thread t1( [&] () { fill(q); } );
    std::thread t2( [&] () { fill(q); } );
    
    t1.join();
    
    std::cout << "front: " << (q.front() ? *q.front() : -1) << '\n';
    
    t2.join();
    
    auto f = std::async(std::launch::async, [&] () { return clear(q); } );
    
    auto sp = f.get();
    
    std::cout << "sp:    " << (sp ? *sp : -1) << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// front: 0
// sp:    23
// Program ended with exit code: 0
