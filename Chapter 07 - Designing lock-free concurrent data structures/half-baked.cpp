#include <iostream>

namespace lf {
template <typename T>
class stack {
public:
    void push(const T &data)
    {
        counted_node_ptr new_node;
        
        new_node.ptr_ = new node(data);
        new_node.external_count_ = 1;
        
        new_node.ptr_->next_ = head_.load();
        while(!head_.compare_exchange_weak(new_node.ptr_->next_, new_node));
    }
    
    // std::shared_ptr<T> pop();
    
    // pop() not even declared, let alone implemented...
    // ~stack() { while(pop()); }
private:
    struct node; // forward declaration

    struct counted_node_ptr {
        int external_count_;
        node *ptr_;
    };

    struct node {
        std::shared_ptr<T> data_;
        std::atomic<int> internal_count;
        counted_node_ptr next_;
        
        node(const T &data) : data_(std::make_shared<T>(data)), internal_count(0) { }
    };
    
    std::atomic<counted_node_ptr> head_;
};
} // namespace lf

int main()
{
    lf::stack<int> stk;
    
    stk.push(1);
    
    return 0;
}
