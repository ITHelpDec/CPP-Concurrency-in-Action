#include <iostream>
#include <future>

namespace lf {
template <typename T>
class stack {
public:
    template <typename V>
    void push(V &&val)
    {
        node *new_node = new node(std::forward<V>(val));
        new_node->next_ = head_.load();
        while (!head_.compare_exchange_weak(new_node->next_, new_node));
    }
    
    void pop(T &val)
    {
        if (!head_) { return; }
        ++threads_in_pop;
        
        node *original_head = head_.load();
        while (!head_.compare_exchange_weak(original_head, original_head->next_));
        
        // std::exchange(val, *original_head->data_);
        val = std::move(*original_head->data_);
        
        try_reclaim(original_head);
    }
    
    std::shared_ptr<T> pop()
    {
        if (!head_) { return {}; }
        ++threads_in_pop;
        
        node *original_head = head_.load();
        while (!head_.compare_exchange_weak(original_head, original_head->next_));
        
        std::shared_ptr<T> result;
        result.swap(original_head->data_);
        
        // ...or?
        // std::exchange(res, original_head);
        
        // why not just steal the data?
        // std::shared_ptr<T> res(std::move(original_head->data_));
        
        try_reclaim(original_head);
        return result;
    }
    
private:
    struct node {
        std::shared_ptr<T> data_;
        node *next_;
        
        template <typename D>
        node(D &&data) : data_(std::make_shared<T>(std::forward<D>(data))), next_(nullptr) { }
    };
    
    std::atomic<node*> head_;
    
    std::atomic<std::size_t> threads_in_pop;
    
    std::atomic<node*> to_be_deleted;
    void try_reclaim(node *old_head)
    {
        if (threads_in_pop == 1) {
            node *nodes_to_delete = to_be_deleted.exchange(nullptr);
            
            // only thread in pop?
            if (!--threads_in_pop)      { delete_nodes(nodes_to_delete); }
            else if (nodes_to_delete)   { chain_pending_nodes(nodes_to_delete); }
            
            delete old_head;
        } else {
            chain_pending_node(old_head);
            --threads_in_pop;
        }
    }
    
    // not sure on the naming of these variables...
    static void delete_nodes(node *nodes)
    {
        while (nodes) {
            node *next = nodes->next_;
            delete nodes;
            nodes = next;
        }
    }
    
    void chain_pending_nodes(node *nodes)
    {
        node *last = nodes;
        while (node *next = last->next_) {  last = next; }
        chain_pending_nodes(nodes, last);
    }
    
    void chain_pending_nodes(node *top, node *bottom)
    {
        // data race?
        bottom->next_ = to_be_deleted; // no .load()?
        while (!to_be_deleted.compare_exchange_weak(bottom->next_, top));
    }
    
    void chain_pending_node(node *n)
    {
        chain_pending_nodes(n, n);
    }
};
}

constexpr std::size_t num_threads = 10000;

void fill(lf::stack<int> &stk) {
    std::vector<std::future<void>> threads(num_threads);
    
    for (int i = 0; i != num_threads; ++i) {
        threads[i] = std::async(std::launch::async, [&, i] () { stk.push(i); });
    }
    
    for (auto &f : threads) { f.get(); }
}

void clear(lf::stack<int> &stk, int &val) {
    std::vector<std::future<void>> threads(num_threads);
    
    for (auto &f : threads) {
        f = std::async(std::launch::async, [&] () { stk.pop(val); });
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
    
    t1.join();
    t2.join();
    
    int val;
    std::shared_ptr<int> sp;
    
    std::thread t3([&] () { clear(stk, val); });
    auto f = std::async(std::launch::async, [&] () { return shared_clear(stk); });
    
    t3.join();
    sp = f.get();
    
    std::cout << "val: " << val << '\n';
    std::cout << "sp:  " << sp << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// val: 110
// sp:  0x60000020c038
// Program ended with exit code: 0
