#include <atomic>
#include <iostream>
#include <thread>
#include <future>

const std::size_t max_hp = 100;

struct hp {
    std::atomic<std::thread::id> id_;
    std::atomic<void*> p_;
}; // hazard pointer

hp hps[max_hp];

class hp_owner {
public:
    hp_owner(const hp_owner&) = delete;
    hp_owner& operator=(const hp_owner&) = delete;
    
    hp_owner() : hp_(nullptr)
    {
        for (std::size_t i = 0; i != max_hp; ++i) {
            std::thread::id old_id;
            if (hps[i].id_.compare_exchange_strong(old_id, std::this_thread::get_id())) {
                hp_ = &hps[i];
                break;
            }
        }
        
        if (hp_) { throw std::runtime_error("No hazard pointers available."); }

    }
    
    ~hp_owner()
    {
        hp_->p_.store(nullptr);
        hp_->id_.store(std::thread::id());
    }
    
    std::atomic<void*>& get_ptr() const { return hp_->p_; }
    
private:
    hp *hp_;
};

std::atomic<void*>& get_hp_for_current_thread() {
    thread_local static hp_owner hazard;
    return hazard.get_ptr();
}

bool outstanding_hps_for(void *p) {
    for (std::size_t i = 0; i != max_hp; ++i)
        if (hps[i].p_.load() == p) { return true; }
    return false;
}

template <typename T>
void do_delete(void *p) { delete static_cast<T*>(p); }

struct data_to_reclaim {
    void *data_;
    std::function<void(void*)> deleter_;
    data_to_reclaim *next_;
    
    // template <typename T>
    // data_to_reclaim(T *p) : data_(p), deleter_(&do_delete<T>), next_(0) { }
    // why next_(0)??? it's a pointer...
    
    template <typename T>
    data_to_reclaim(T *p) : data_(p), deleter_(&do_delete<T>), next_(nullptr) { }
    
    ~data_to_reclaim()  { deleter_(data_); }
};

std::atomic<data_to_reclaim*> nodes_to_reclaim;

void add_to_reclaim_list(data_to_reclaim *node) {
    node->next_ = nodes_to_reclaim.load();
    while (!nodes_to_reclaim.compare_exchange_weak(node->next_, node));
}

void delete_nodes_with_no_hps() {
    data_to_reclaim *current = nodes_to_reclaim.exchange(nullptr);
    
    while (current) {
        data_to_reclaim *next = current->next_;
        
        if (!outstanding_hps_for(current->data_)) { delete current; }
        else { add_to_reclaim_list(current); }
        
        current = next;
        
        // not the typical `current = current->next_`, becaue we maybe have deleted it already
    }
}

// raw new
template <typename T>
void reclaim_later(T *data) { add_to_reclaim_list(new data_to_reclaim(data)); }

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
    
    std::shared_ptr<T> pop()
    {
        if (!head_) { return std::shared_ptr<T>(); }
        
        // Non-const lvalue reference to type 'atomic<...>' cannot bind to a temporary of type 'atomic<...>'
        std::atomic<void*> &hp = get_hp_for_current_thread();
        node *old_head = head_.load();
        
        // opting for strong because we're performing work inside the CAS
        while (!head_.compare_exchange_strong(old_head, old_head->next_)) {
            node *temp;
            while (old_head != temp) {
                temp = old_head;
                hp.store(old_head);
                old_head = head_.load();
            }
        }
        
        hp.store(nullptr);
        
        std::shared_ptr<T> result;
        
        if (old_head) {
            result.swap(old_head->data_);
            
            if (outstanding_hps_for(old_head)) {
                reclaim_later(old_head);
            } else {
                delete old_head;
            }
            
            delete_hazardfree();
        }
        
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
    
    std::atomic<void*> get_hp()
    {
        // another half-baked example
        return std::atomic<void*>();
    }
    
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
    
    void delete_hazardfree()
    {
        std::cout << "waste of time\n";
    }
};
} // namespace lf (lock-free)

int main()
{
    lf::stack<int> stk;
    
    std::thread t([&] () {
        stk.push(1);
    });
    
    std::shared_ptr<int> sp;
    auto f = std::async(std::launch::async, [&] () {
        return stk.pop();
    });
    
    t.join();
    f.get();
    
    std::cout << "sp: " << sp << '\n';
    
    return 0;
}
