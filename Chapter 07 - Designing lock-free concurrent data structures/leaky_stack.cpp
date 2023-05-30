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
        val = old_head->data_;
    }
    
    void print()
    {
        while (head_) {
            node *current = head_.load();
            while (!head_.compare_exchange_weak(current, current->next_));
            std::cout << current->data_ << ' ';
        } std::cout << '\n';
    }
    
private:
    struct node {
        T data_;
        node *next_;
        
        template <typename D>
        node(D &&data) : data_(std::forward<D>(data)), next_(nullptr)  { }
    };
    
    std::atomic<node*> head_;
};
} // namespace lf

constexpr std::size_t num_threads = 1000;

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

int main()
{
    lf::stack<int> stk;
    
    fill(stk);
    
    int val;
    std::cout << "stk: ";
    std::thread t1([&] () { clear(stk, val); });
    std::thread t2([&] () { stk.print(); });
    
    t1.join();
    t2.join();
    
    std::cout << "val: " << val << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// stk: 999 998 ... 9 8 3 7 4 1 0 (a bit funky...)
// val: 5
