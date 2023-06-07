#include <iostream>
 
namespace lf {
template <typename T>
class queue {
public:
    template <typename V>
    void push(V &&val)
    {
        auto new_data = std::make_unique<T>(std::forward<V>(val));
        
        counted_node_ptr new_next{ 1, new node };
        
        counted_node_ptr old_tail = tail_.load();
        
        for (;;) {
            // half_based...
            increase_external_count(tail_, old_tail);
            
            T *old_data = nullptr;
            
            if (old_tail.ptr_->data_.compare_exchange_strong(old_data, new_data.get())) {
                old_tail.ptr_->next_ = new_next;
                old_tail = tail_.exchange(new_next);
                
                // ..., half-baked...
                free_external_counter(old_tail);
                
                new_data.release();
                break;
            }
            
            // ...and also half-baked
            old_tail.ptr_->release_ref();
        }
    }
    
private:
    struct node;
    struct counted_node_ptr {
        int external_count_;
        node *ptr_;
    };
    
    std::atomic<counted_node_ptr> head_;
    std::atomic<counted_node_ptr> tail_;
    
    struct node_counter {
        unsigned internal_count_ : 30;
        unsigned external_counters_ : 2;
    };
    
    struct node {
        std::atomic<T*> data_;
        std::atomic<node_counter> count_;
        counted_node_ptr next_;
        
        node () : next_({ 0, nullptr})
        {
            count_.store({ 0, 2 });
        }
        
        void release_ref() { std::cout << "again...does nothing...\n"; }
    };
    
    void increase_external_count(const std::atomic<counted_node_ptr> &tail_, const counted_node_ptr &old_tail)
    {
        std::cout << "does nothing...\n";
    }
    
    void free_external_counter(const counted_node_ptr &old_tail)
    {
        std::cout << "also does nothing...\n";
    }
};
}
 
 int main()
{
     lf::queue<int> q;
     
     q.push(1);
     
     return 0;
 }

// Thread 1: EXC_BAD_ACCESS (code=1, address=0x0)
// template<class _Tp>
// _LIBCPP_INLINE_VISIBILITY
// bool __cxx_atomic_compare_exchange_strong(__cxx_atomic_base_impl<_Tp> * __a, _Tp* __expected, _Tp __value, memory_order __success, memory_order __failure) _NOEXCEPT {
//     return __c11_atomic_compare_exchange_strong(&__a->__a_value, __expected, __value, static_cast<__memory_order_underlying_t>(__success), static_cast<__memory_order_underlying_t>(__to_failure_order(__failure)));
