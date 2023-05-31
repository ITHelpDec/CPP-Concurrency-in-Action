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
        node *old_head = head_.load();
        while (!head_.compare_exchange_weak(old_head, old_head->next_));
        val = *old_head->data_.get();
    }
    
    std::shared_ptr<T> pop()
    {
        if (!head_) { return std::shared_ptr<T>(); }
        
        node *old_head = head_.load();
        while (!head_.compare_exchange_weak(old_head, old_head->next_));
        return old_head->data_;
    }
    
    void print()
    {
        while (head_) {
            node *current = head_.load();
            while (!head_.compare_exchange_weak(current, current->next_));
            std::cout << (current->data_ ? *current->data_.get() : -1) << ' ';
        } std::cout << '\n';
    }
    
private:
    struct node {
        std::shared_ptr<T> data_;
        node *next_;
        
        template <typename D>
        node(D &&data) : data_(std::make_shared<T>(std::forward<D>(data))), next_(nullptr)  { }
    };
    
    std::atomic<node*> head_;
};
} // namespace lf

constexpr std::size_t num_threads = 100;

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

// std::cout from print() seems to mess up returns
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
    
    std::cout << "stk: ";
    std::thread t3([&] () { clear(stk, val); });
    auto f = std::async(std::launch::async, [&] () { return shared_clear(stk); });
    std::thread t4([&] () { stk.print(); });
    
    t3.join();
    sp = f.get();
    t4.join();
    
    std::cout << "sp:  " << (sp ? *sp.get() : -1) << '\n';
    std::cout << "val: " << val << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// stk: 99 99 98 98 97 97 ... 10 10 9 8 7 7 5 6 3 1 6 4 4 3 2 0 0 (funky again...)
// sp:  -1 (affected by print
// val: 2
