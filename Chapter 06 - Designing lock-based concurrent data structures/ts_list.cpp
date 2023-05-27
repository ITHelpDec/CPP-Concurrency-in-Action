#include <mutex>
#include <memory>
#include <iostream>
#include <thread>
#include <random>

namespace ts {
template <typename T>
class list {
public:
    list() { }
    
    ~list()
    {
        remove_if( [] (const node&) { return true; } );
    }
    
    list(const T&) = delete;
    list& operator=(const T&) = delete;
    
    void push_front(const T &value)
    {
        // std::unique_ptr<node> new_node(new node(value));
        auto new_node = std::make_unique<node>(value);
        
        std::lock_guard lock(head_.m_);
        new_node->next_ = std::move(head_.next_);
        head_.next_ = std::move(new_node);
    }
    
    template <typename Func>
    void for_each(Func f)
    {
        node *current = &head_;
        std::unique_lock lock(head_.m_);
        
        // deliberate assignment operator (for as long as node is valid)
        while (node *next = current->next_.get()) {
            // create inner lock
            std::unique_lock next_lock(next->m_);
            
            // pause outer lock
            lock.unlock();
            
            // do business
            f(*next->data_);
            current = next;
            
            // transfer inner lock to outer
            lock = std::move(next_lock);
        }
    }
    
    template <typename Pred>
    std::shared_ptr<T> find_first_if(Pred p)
    {
        node *current = &head_;
        std::unique_lock lock(head_.m_);
        
        // similar strategy
        while(node *next = current->next_.get()) {
            std::unique_lock next_lock(next->m_);
            lock.unlock();
            
            if (p(*next->data_)) { return next->data_; }
            
            current = next;
            
            lock = std::move(next_lock);
        }
        
        return std::shared_ptr<T>();
    }
    
    template <typename Pred>
    void remove_if(Pred p)
    {
        node *current = &head_;
        std::unique_lock lock(head_.m_);
        
        while (node *next = current->next_.get()) {
            std::unique_lock next_lock(next->m_);
            
            // p: unlink; !p: move on
            if (p(*next->data_)) {
                std::unique_ptr<node> old_next = std::move(current->next_);
                current->next_ = std::move(next->next_);
                next_lock.unlock();
            } else {
                lock.unlock();
                current = next;
                lock = std::move(next_lock);
            }
        }
    }
    
private:
    struct node {
        node() : next_() { }
        node(const T &value) : data_(std::make_shared<T>(value)) { }
        
        std::mutex m_;
        std::shared_ptr<T> data_;
        std::unique_ptr<node> next_;
    };
    
    node head_;
};
} // namespace ts

int num() {
    static std::default_random_engine e;
    static std::uniform_int_distribution u(1, 10);
    return u(e);
}

int main()
{
    std::cout << "Initialising and populating list...\n";
    
    ts::list<int> tslist;
    for (int i = 0; i != 10; ++i) { tslist.push_front(i); }
    
    std::cout << "List: ";
    tslist.for_each( [&] (int n) {std::cout << n << ' '; } );
    std::cout << "\n\n";
    
    int p = 3;
    std::cout << "Finding first number less than " << p << "...";
    auto first = tslist.find_first_if( [&] (int i) { return i < p; } );
    
    std::cout << *first.get() << "\n\n";
    
    std::cout << "Squaring every element of our list...\n";
    tslist.for_each( [&] (int &n) { n *= 2; } );
    
    std::cout << "List: ";
    tslist.for_each( [&] (int n) { std::cout << n << ' '; } );
    std::cout << "\n\n";
    
    int l = 3;
    std::cout << "Removing every element divisible by " << l << "...\n";
    tslist.remove_if( [&] (int n) { return n % l == 0; } );
    
    std::cout << "List: ";
    tslist.for_each( [&] (int n) {std::cout << n << ' '; } );
    std::cout << "\n\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // MULTITHREADING  - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::size_t times = 100;
    
    std::cout << "Launching threads to print out two predicates while populating, modifying and erasing from our list...\n\n";
    
    std::thread t1([&] () {
        for (int i = 0; i != times; ++i) {
            tslist.push_front(num());
        }
    });
    
    std::thread t2([&] () {
        for (int i = 0; i != times; ++i) {
            tslist.remove_if([&] (int n) { return n % 3 == 0; } );
        }
    });
    
    std::thread t3([&] () {
        for (int i = 0; i != times; ++i) {
            tslist.for_each([&] (int &n) { n += 2; } );
        }
    });
    
    std::thread t4([&] () {
        for (int i = 0; i != times; ++i) {
            auto sp = tslist.find_first_if([&] (int n) { return n > 0; } );
            std::cout << "-" << (sp.get() ? *sp.get() : -1) << "- ";
        }
    });
    
    std::thread t5([&] () {
        for (int i = 0; i != times; ++i) {
            auto sp = tslist.find_first_if([&] (int n) { return n > 5; } );
            std::cout << "_" << (sp.get() ? *sp.get() : -2) << "_ ";
        }
    });
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising and populating list...
// List: 9 8 7 6 5 4 3 2 1 0

// Finding first number less than 3...2

// Squaring every element of our list...
// List: 18 16 14 12 10 8 6 4 2 0

// Removing every element divisible by 3...
// List: 16 14 10 8 4 2

// Launching threads to print out two predicates while populating, modifying and erasing from our list...

// _18_ _16_ _16_ _18_ _18_ -18_18_ _18_ - _11_ _11_ _11_ _11_ _9_ -5- _10_ -5- _10_
// -5- -5- _10_ _10_ -5- _10_ _10_ _9_ _9_ _7_ _6_ _6_ _6_ -6- _6_ -6- _6_ _6_ _8_ -8-
// -8- -8- -8- _8_ _8_ -6- _10_ -9- _11_ -2- _9_ -2- -2- -4- -4- -10- -3- -5- -4- _9_
// _11_ -19- _10_ -12- -12- _12_ _14_ -14- _14_ _16_ _16_ -16- -18- -18- -2- -7- -9-
// _30_ _23_ _33_ _43_ _47_ _49_ _51_ _55_ _65_ _75_ -79- -83- -85- _85_ -85- -85- _85_
// _89_ _89_ -95- _99_ _103_ -105- _111_ -113- _117_ -121- _123_ _127_ -131- -135- _137_
// _141_ -145- -149- _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_
// _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_
// _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ _151_ -151- -151-
// -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151-
// -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151-
// -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151- -151-
// -151- -151- -151- -151- -151- -151- -151- -151- -151- -151-
// Program ended with exit code: 0
