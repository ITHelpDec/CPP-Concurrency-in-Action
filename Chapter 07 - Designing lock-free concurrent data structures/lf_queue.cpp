#include <iostream>
#include <future>

namespace lf {
template <typename T>
class queue {
public:
    queue() : head_(new node), tail_(head_.load()) { }
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    ~queue()
    {
        while (node *old_head = head_.load()) {
            head_.store(old_head->next_);
            delete old_head;
        }
    }
    
    template <typename V>
    void push(V &&val)
    {
        auto new_data = std::make_shared<T>(std::forward<V>(val));
        
        node *p = new node;
        node *old_tail = tail_.load();
        
        old_tail->data_.swap(new_data);
        old_tail->next_ = p;
        
        tail_.store(p);
    }
    
    std::shared_ptr<T> pop()
    {
        // head moved within pop_haed()
        node *old_head = pop_head();
        
        if (!old_head) { return std::shared_ptr<T>(); }
        
        auto result = old_head->data_;
        
        delete old_head;
        
        return result;
    }
    
    std::shared_ptr<T> front()
    {
        node *current = head_.load();
        return current->data_;
    }
    
private:
    struct node {
        std::shared_ptr<T> data_;
        node *next_;
        
        node() : next_(nullptr) { }
    };
    
    std::atomic<node*> head_;
    std::atomic<node*> tail_;
    
    node* pop_head()
    {
        node *old_head = head_.load();
        
        if (old_head == tail_.load()) { return nullptr; }
        
        head_.store(old_head->next_);
        return old_head;
    }
};
} // namespace lf (lock-free)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

constexpr std::size_t sz = 100;

template <typename T>
void fill(lf::queue<T> &q) {
    std::vector<std::thread> threads(sz);
    
    for (std::size_t i = 0; i != sz; ++i) {
        threads[i] = std::thread( [&, i] () { q.push(i); } );
    }
    
    for (auto &t : threads) { t.join(); }
}

template <typename T>
std::shared_ptr<T> clear(lf::queue<T> &q) {
    std::vector<std::future<std::shared_ptr<T>>> threads(sz);
    
    for (std::size_t i = 0; i != sz; ++i) {
        threads[i] = std::async(std::launch::async, [&, i] () { return q.pop(); } );
    }
    
    std::shared_ptr<T> result;
    
    for (auto &fut : threads) { result = fut.get(); }
    
    return result;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    lf::queue<int> q;
    
    std::thread t( [&] () { fill(q); } );
    
    std::cout << "front: " << (q.front() ? *q.front() : -1) << '\n';
    
    auto f = std::async(std::launch::async, [&] () { return clear(q); } );
    
    t.join();
    auto sp = f.get();
    
    std::cout << "sp:    " << (sp ? *sp : -1) << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// front: -1
// sp:    91
// Program ended with exit code: 0
