#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <future>
#include <random>
#include <iostream>

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
        std::unique_ptr<node> old_head = pop_head();
        return old_head ? old_head->data_ : std::shared_ptr<T>();
    }
    
    template <typename V>
    void push(V &&val)
    {
        auto new_data = std::make_shared<T>(std::forward<V>(val));
        auto p = std::make_unique<node>();
        node *new_tail = p.get();
        
        // lock the tail
        std::lock_guard lock(tail_m);
        
        tail_->data_ = new_data;
        tail_->next_ = std::move(p);
        tail_ = new_tail;
    }
    
private:
    struct node
    {
        std::shared_ptr<T> data_;
        std::unique_ptr<node> next_;
    };
    
    std::unique_ptr<node> pop_head()
    {
        // lock the head
        std::lock_guard lock(head_m);
        if (head_.get() == get_tail()) { return nullptr; }
        std::unique_ptr<node> old_head = std::move(head_);
        head_ = std::move(old_head->next_);
        return old_head;
    }
    
    node* get_tail()
    {
        std::lock_guard lock(tail_m);
        return tail_;
    }
    
    std::unique_ptr<node> head_;
    node *tail_;
    
    std::mutex head_m, tail_m;
};
} // namespace mt (multi-threaded)

int main()
{
    std::cout << "Initialising concurrent queue...\n";
    mt::queue<int> q;
    
    const std::size_t num_threads = std::thread::hardware_concurrency();
    
    std::vector<std::thread> threads(num_threads);
    std::vector<std::future<std::shared_ptr<int>>> futures(num_threads);
    
    std::vector<std::shared_ptr<int>> results;
    
    std::cout << "Launching threads...";
    for (int i = 0; i != threads.size(); ++i) {
        threads[i] = std::thread( [&, i] () { q.push(num()); } );
    }
    
    std::cout << "launching futures...";
    for (int i = 0; i != futures.size(); ++i) {
        futures[i] = std::async(std::launch::async, [&, i] () { return q.try_pop(); } );
    }
    
    std::cout << "joining 'em all up...\n";
    for (auto &t : threads) { t.join(); }
    for (auto &f : futures) { results.emplace_back(f.get()); }
    
    std::cout << "Results: ";
    for (const auto &sp : results) { std::cout << *sp.get() << ' '; }
    std::cout << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising concurrent queue...
// Launching threads...launching futures...joining 'em all up...
// Results: 4 3 3 2 2 4 2 2
// Program ended with exit code: 0
