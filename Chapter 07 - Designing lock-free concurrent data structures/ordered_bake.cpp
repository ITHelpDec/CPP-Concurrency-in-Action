#include <iostream>
#include <future>

namespace lf {
template <typename T>
class stack {
public:
    template <typename D>
    void push(D &&data)
    {
        counted_node_ptr new_node{ new node(std::forward<D>(data)), 0 };
        new_node.ptr_->next_ = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(new_node.ptr_->next_, new_node,
                                            std::memory_order_release, std::memory_order_relaxed));
    }
    
    std::shared_ptr<T> pop()
    {
        counted_node_ptr old_head = head_.load(std::memory_order_relaxed);
        
        for(;;) {
            increase_head_count(old_head);
            
            node *ptr = old_head.ptr_;
            
            if (!ptr->next_.ptr_) { return std::shared_ptr<T>(); }
            
            if (head_.compare_exchange_strong(old_head, ptr->next_, std::memory_order_relaxed)) {
                std::shared_ptr<T> result;
                result.swap(old_head.ptr_->data_);
                
                int count_increase = old_head.external_count_ - 2;
                
                if (ptr->internal_count_.fetch_add(count_increase, std::memory_order_release) == -count_increase)
                {
                    delete ptr;
                }
                
                return result;
            }
            
            else if (ptr->internal_count_.fetch_add(-1, std::memory_order_relaxed) == 1)
            {
                ptr->internal_count_.load(std::memory_order_acquire);
                delete ptr;
            }
        }
    }
    
    ~stack() { while(pop()); }
    
private:
    struct node;
    struct counted_node_ptr {
        node *ptr_;
        int external_count_;
    };
    
    struct node {
        std::shared_ptr<T> data_;
        counted_node_ptr next_;
        
        std::atomic<int> internal_count_;
        
        template <typename D>
        node(D &&data) : data_(std::make_shared<T>(std::forward<D>(data))), internal_count_(0) { }
    };
    
    std::atomic<counted_node_ptr> head_;
    
    void increase_head_count(counted_node_ptr& old_counter)
    {
        counted_node_ptr new_counter;
        
        do {
            new_counter = old_counter;
            ++new_counter.external_count_;
        } while(!head_.compare_exchange_strong(old_counter, new_counter,
                                               std::memory_order_acquire, std::memory_order_relaxed));
        // MUST be a "do while", and not a "while"
        
        old_counter.external_count_ = new_counter.external_count_;
    }
};
} // namespace lf (lock-free)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

// sp1: 5109
// sp2: 9105
// Program ended with exit code: 0
