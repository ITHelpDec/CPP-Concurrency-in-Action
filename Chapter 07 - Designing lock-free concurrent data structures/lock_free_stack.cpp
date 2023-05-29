#include <atomic>
#include <memory>
#include <iostream>
#include <barrier>
#include <vector>
#include <thread>
#include <random>

int num() {
    static std::default_random_engine e;
    static std::uniform_int_distribution u(1, 10);
    return u(e);
}

namespace lf {
template <typename T>
class stack {
public:
    template <typename D>
    void push(D &&data)
    {
        // Cannot assign to variable 'new_node' with const-qualified type 'const lf::stack<int>::node *'
        // const node *new_node = new node(data);
        
        // positioning of const...pointer isn't constant, contents are
        node* const new_node = new node(std::forward<D>(data));
        new_node->next_ = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(new_node->next_, new_node)) { }
        
        // unsuccessful attempt to use smarter pointers
        // auto new_node = std::make_unique<node>(data);
        // new_node->next_ = std::unique_ptr<node>(std::move(head_.load()));
        
        // No matching member function for call to 'compare_exchange_weak'
        // while (!head_.compare_exchange_weak(new_node->next_, new_node)) { }
    }
    
    void print()
    {
        node *current = head_.load();
        
        while (current) {
            std::cout << current->data_ << ' ';
            current = current->next_;
        } std::cout << '\n';
    }
private:
    struct node {
        T data_;
        node *next_;
        node(const T& data) : data_(data) { }
        
        // std::shared_ptr<T> data_;
        // std::unique_ptr<node> next_;
        // node(const T& data) : data_(std::make_shared<T>(data)) { }
    };
    
    std::atomic<node*> head_;
};
} // namespace lf (lock-free)

void test(lf::stack<int> &lfstk) {
    std::barrier sync_then_push(4);
    
    std::vector<std::thread> threads(4);
    
    for (int i = 0; i != 4; ++i) {
        threads[i] = std::thread( [&, i] () {
            sync_then_push.arrive_and_wait();
            lfstk.push(num());
            
            int x = num();
            sync_then_push.arrive_and_wait();
            lfstk.push(x);
        });
    }
    
    for (auto &t : threads) { t.join(); }
}

int main()
{
    std::cout << "lfstk: ";
    lf::stack<int> lfstk;
    
    test(lfstk);
    
    lfstk.print();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// lfstk: 6 3 1 9 1 1 1 2
// Program ended with exit code: 0
