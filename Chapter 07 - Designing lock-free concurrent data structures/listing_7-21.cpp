#include <iostream>
#include <thread>

namespace lf {
template <typename T>
class queue {
public:
    queue() : head_({ 2, new node }), tail_(head_.load()) { }
    
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;
    
    template <typename V>
    void push(V &&val)
    {
        auto new_data = std::make_unique<T>(std::forward<V>(val));
        
        counted_node_ptr new_next{ 1, new node };
        
        counted_node_ptr old_tail = tail_.load();
        
        for (;;) {
            increase_external_count(tail_, old_tail);
            
            T *old_data = nullptr;
            
            if (old_tail.ptr_->data_.compare_exchange_strong(old_data, new_data.get())) {
                old_tail.ptr_->next_ = new_next;
                old_tail = tail_.exchange(new_next);
                
                free_external_counter(old_tail);
                
                new_data.release();
                break;
            }
            
            old_tail.ptr_->release_ref();
        }
    }
    
    std::unique_ptr<T> pop()
    {
        counted_node_ptr old_head = head_.load(std::memory_order_relaxed);
        
        for (;;) {
            increase_external_count(head_, old_head);
            
            node *oh_ptr = old_head.ptr_;
            if (oh_ptr == tail_.load().ptr_) { return {}; }
            
            counted_node_ptr oh_next = oh_ptr->next_.load();
            
            if (head_.compare_exchange_strong(old_head, oh_next)) {
                T *result = oh_ptr->data_.exchange(nullptr);
                free_external_counter(old_head);
                return std::unique_ptr<T>(result);
            }
            
            oh_ptr->release_ref();
        }
    }
    
    ~queue() { while (pop()); }
    
private:
    struct node;
    struct counted_node_ptr {
        int external_count_;
        node *ptr_;
    };
    
    struct node_counter {
        std::size_t internal_count_ : 30;
        std::size_t external_counters_ : 2;
    };
    
    struct node {
        std::atomic<T*> data_;
        std::atomic<node_counter> count_;
        std::atomic<counted_node_ptr> next_;
        
        // no constructor in listing?
        // no constructor causes godbolt to hang
        node ()
        {
            data_.store(nullptr);
            count_.store(node_counter{ 0, 2 });
            next_.store({ 0, nullptr });
        }
        
        // release_ref() not included in listing
        void release_ref()
        {
            node_counter old_counter = count_.load(std::memory_order_relaxed);
            node_counter new_counter;
            
            do {
                new_counter = old_counter;
                --new_counter.internal_count_;
            } while (!count_.compare_exchange_strong(old_counter, new_counter, std::memory_order_acq_rel, std::memory_order_relaxed));
            
            if (!new_counter.internal_count_ && !new_counter.external_counters_) { delete this; }
        }
    };
    
    std::atomic<counted_node_ptr> head_;
    std::atomic<counted_node_ptr> tail_;
    
    static void increase_external_count(std::atomic<counted_node_ptr> &counter, counted_node_ptr &old_number)
    {
        counted_node_ptr new_counter;
        
        do {
            new_counter = old_number;
            ++new_counter.external_count_;
        } while (!counter.compare_exchange_strong(old_number, new_counter, std::memory_order_acq_rel, std::memory_order_relaxed));
        
        old_number.external_count_ = new_counter.external_count_;
    }
    
    static void free_external_counter(counted_node_ptr &old_node_ptr)
    {
        node *on_ptr = old_node_ptr.ptr_;
        
        int count_increase = old_node_ptr.external_count_ - 2;
        
        node_counter old_counter = on_ptr->count_.load(std::memory_order_relaxed);
        node_counter new_counter;
        
        do {
            new_counter = old_counter;
            --new_counter.external_counters_;
            new_counter.internal_count_ += count_increase;
        } while (!on_ptr->count_.compare_exchange_strong(old_counter, new_counter, std::memory_order_acq_rel, std::memory_order_relaxed));
        
        if (!new_counter.internal_count_ && !new_counter.external_counters_) { delete on_ptr; }
    }
    
};
} // namespace lf (lock-free)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

lf::queue<int> q;

const int consumers = 5;
const int reps = 1000;

void producer() {
    for (int i = 0; i != reps * consumers; ++i) { q.push(i); }
}

void consumer() {
    for (int i = 0; i != reps; ++i) {
        std::unique_ptr<int> p;
        
        for (;;) {
            p = q.pop();
            if (p) { break; }
            else {
                //std::cout << "empty" << std::endl;
                std::this_thread::yield();
            }
        } // std::cout << *p << '\n';
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main() {
    
    std::thread p(producer);
    
    std::vector<std::thread> c;
    for (int i = 0; i != consumers; ++i) { c.emplace_back(consumer); }
    
    p.join();
    
    for (auto &cc : c) { cc.join(); }
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Program ended with exit code: 0
