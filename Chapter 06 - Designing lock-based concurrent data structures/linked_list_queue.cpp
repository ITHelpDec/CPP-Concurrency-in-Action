#include <memory>
#include <iostream>

namespace st {
template <typename T>
class queue {
public:
    queue() : tail_(nullptr) { }
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    std::shared_ptr<T> try_pop()
    {
        if (!head_) { return std::shared_ptr<T>(); }
        
        auto result = std::make_shared<T>(std::move(head_->data_));
        
        std::unique_ptr<node> old_head = std::move(head_);
        head_ = std::move(old_head->next_);
        
        if (!head_) { tail_ = nullptr; }
        return result;
    }
    
    // void push(T val)
    template <typename V>
    void push(V &&val)
    {
        // std::unique_ptr<node> p(new node(std::move(new_value)));
        auto p = std::make_unique<node>(std::forward<V>(val));
        
        node *new_tail = p.get();
        
        if (tail_) {
            tail_->next_ = std::move(p);
        } else {
            head_ = std::move(p);
        }
        
        tail_ = new_tail;
    }
    
    bool empty() const { return !head_; } // similar to check in .push()
    
private:
    
    struct node
    {
        T data_;
        std::unique_ptr<node> next_;
        node(T data) : data_(std::move(data)) { }
    };
    
    std::unique_ptr<node> head_;
    node *tail_;
};
} // namespace st (single-thread)

int main()
{
    st::queue<int> q;
    
    std::cout << std::boolalpha;
    
    std::cout << "Attempting to pop from an empty queue...";
    auto sp = q.try_pop();
    std::cout << "pop successful? " << (sp ? "yes" : "no\n");
    if (sp) { std::cout << "(" << *sp.get() << ")\n"; }
    std::cout << "Is our queue empty? " << q.empty() << "\n\n";
    
    std::cout << "Pushing an rvalue...\n";
    q.push(69);
    
    std::cout << "Is our queue now empty? " << q.empty() << "\n\n";
    
    std::cout << "Attempting to pop...";
    sp = q.try_pop();
    std::cout << "pop successful? " << (sp ? "yes" : "no\n");
    if (sp) { std::cout << " (" << *sp.get() << ")\n"; }
    
    std::cout << "Is our queue now empty? " << q.empty() << "\n\n";
    
    std::cout << "Pushing an lvalue...\n";
    int lvalue = 42;
    q.push(lvalue);
    std::cout << "Is our queue now empty? " << q.empty() << "\n\n";
    
    std::cout << "Attempting to pop...";
    sp = q.try_pop();
    std::cout << "pop successful? " << (sp ? "yes" : "no\n");
    if (sp) { std::cout << " (" << *sp.get() << ")\n"; }
    std::cout << "Is our queue now empty? " << q.empty() << "\n\n";
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Attempting to pop from an empty queue...pop successful? no
// Is our queue empty? true
//
// Pushing an rvalue...
// Is our queue now empty? false
//
// Attempting to pop...pop successful? yes (69)
// Is our queue now empty? true
//
// Pushing an lvalue...
// Is our queue now empty? false
//
// Attempting to pop...pop successful? yes (42)
// Is our queue now empty? true
//
// Program ended with exit code: 0
