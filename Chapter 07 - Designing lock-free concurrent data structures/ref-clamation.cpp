#include <iostream>
#include <future>

namespace lf {
template <typename T>
class stack {
public:
    template <typename V>
    void push(V &&val)
    {
        auto new_node = std::make_shared<node>(std::forward<V>(val));
        new_node->next_ = std::atomic_load(&head_);
        while (!std::atomic_compare_exchange_weak(&head_, &new_node->next_, new_node));
    }
    
    std::shared_ptr<T> pop()
    {
        auto original_head = std::atomic_load(&head_);
        while (original_head && !std::atomic_compare_exchange_weak(&head_, &original_head, original_head->next_));
    
        if (original_head) {
            std::atomic_store(&original_head->next_, {} );
            return original_head->data_;
        }
        
        return {};
    }
    
    // clever destructor
    ~stack() { while(pop()); }
    
private:
    struct node {
        std::shared_ptr<T> data_;
        std::shared_ptr<node> next_;
        
        template <typename D>
        node(D &&data) : data_(std::make_shared<T>(std::forward<D>(data))), next_(nullptr) { }
    };
    
    std::shared_ptr<node> head_;
};
} //namespace lock-free

constexpr std::size_t num_threads = 10000;

void fill(lf::stack<int> &stk) {
    std::vector<std::future<void>> threads(num_threads);
    
    for (int i = 0; i != num_threads; ++i) {
        threads[i] = std::async(std::launch::async, [&, i] () { stk.push(i); });
    }
    
    for (auto &f : threads) { f.get(); }
}

std::shared_ptr<int> shared_clear(lf::stack<int> &stk) {
    std::vector<std::future<std::shared_ptr<int>>> threads(num_threads);
    
    for (auto &f : threads) {
        f = std::async(std::launch::async, [&] () { return stk.pop(); });
    }
    
    std::shared_ptr<int> sp;
    for (auto &f : threads) { sp = f.get(); }
    
    return sp;
}

int main()
{
    lf::stack<int> stk;
    
    std::thread t1( [&] () { fill(stk); } );
    std::thread t2( [&] () { fill(stk); } );
    t1.detach();
    t2.detach();
    
    std::shared_ptr<int> sp1, sp2;
    auto f1 = std::async(std::launch::async, [&] () { return shared_clear(stk); });
    auto f2 = std::async(std::launch::async, [&] () { return shared_clear(stk); });
    sp1 = f1.get();
    sp2 = f2.get();
    
    std::cout << "sp1: " << (sp1 ? *sp1 : -1) << '\n';
    std::cout << "sp2: " << (sp2 ? *sp2 : -1) << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// sp1: 9877
// sp2: 9970
// Program ended with exit code: 0
