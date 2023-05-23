#include <memory>
#include <iostream>

namespace st {
template <typename T>
class queue {
public:
    // avoid raw call to operator new
    // queue() : head_(new node), tail_(head_.get()) { }
    queue() : head_(std::make_unique<node>()), tail_(head_.get()) { }
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    std::shared_ptr<T> try_pop()
    {
        if(head_.get() == tail_) { return std::shared_ptr<T>(); }
        
        // std::shared_ptr<T> const res(head_->data_);
        auto result = head_->data_;
        
        std::unique_ptr<node> old_head = std::move(head_);
        head_ = std::move(old_head->next_);
        return result;
    }
    
    // void push(T val)
    template <typename V>
    void push(V &&val)
    {
        // account for l- and r-values and void extra copy
        // std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
        auto new_data = std::make_shared<T>(std::forward<V>(val));
        
        // better shorthand
        // std::unique_ptr<node> p(new node);
        auto p = std::make_unique<node>();
        
        tail_->data_ = new_data;
        node *new_tail = p.get();
        tail_->next_ = std::move(p);
        tail_ = new_tail;
    }
    
    std::shared_ptr<T> front()
    {
        if (head_.get() == tail_) { return std::shared_ptr<T>(); }
        return head_->data_;
    }
    
    bool empty() const
    {
        return head_.get() == tail_;
    }
    
private:
    struct node
    {
        std::shared_ptr<T> data_;
        std::unique_ptr<node> next_;
    };
    
    std::unique_ptr<node> head_;
    node *tail_;
};
} // namespace st (single-thread)

int main()
{
    std::cout << "Initialising dummy-node queue...\n\n";
    st::queue<int> q;
    
    std::cout << "Adding rvalue (42)...";
    q.push(42);
    
    std::cout << *q.front() << " is now at the front of the queue.\n";
    
    int lvalue = 69;
    std::cout << "Adding lvalue (" << lvalue << ")...";
    q.push(lvalue);
    
    std::cout << *q.front() << " is still at the front of the queue.\n\n";
    
    std::cout << "Attempting to pop " << *q.front() << " from the front of the queue...\n";
    std::shared_ptr<int> sp = q.try_pop();
    
    std::cout << "The value we popped was " << *sp.get() << ".\n";
    std::cout << *q.front() << " from earlier is now at the front of the queue.\n\n";
    
    std::cout << "Attempting to pop again...\n";
    sp = q.try_pop();
    
    std::cout << std::boolalpha;
    std::cout << "The value we popped this time was " << *sp.get() << ".\n";
    std::cout << "Is our queue now empty? " << (q.empty() ? "Yes\n" : "No\n");
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising dummy-node queue...

// Adding rvalue (42)...42 is now at the front of the queue.
// Adding lvalue (69)...42 is still at the front of the queue.

// Attempting to pop 42 from the front of the queue...
// The value we popped was 42.
// 69 from earlier is now at the front of the queue.

// Attempting to pop again...
// The value we popped this time was 69.
// Is our queue now empty? Yes
// Program ended with exit code: 0
