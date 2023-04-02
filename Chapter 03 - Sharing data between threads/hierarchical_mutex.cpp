#include <mutex>
#include <iostream>

class hierarchical_mutex {
public:
    explicit hierarchical_mutex(unsigned long value) : hierarchy_value(value), previous_hierarchu_value(0) { }
    
    void lock()
    {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchical_value();
    }
    
    void unlock()
    {
        if (this_thread_hierarchy_value != hierarchy_value) {
            throw std::logic_error("\"mutex hierarchy violated\"\n");
        }
        
        this_thread_hierarchy_value = previous_hierarchu_value;
        internal_mutex.unlock();
    }
    
    bool try_lock()
    {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock()) { return false; }
        update_hierarchical_value();
        return true;
    }
private:
    std::mutex internal_mutex;
    
    const unsigned long hierarchy_value;
    unsigned long previous_hierarchu_value;
    
    static thread_local unsigned long this_thread_hierarchy_value;
    
    void check_for_hierarchy_violation()
    {
        if (this_thread_hierarchy_value <= hierarchy_value) {
            throw std::logic_error("\"mutex hierarchy violated\"\n");
        }
    }
    
    void update_hierarchical_value()
    {
        previous_hierarchu_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }
};

thread_local unsigned long hierarchical_mutex::this_thread_hierarchy_value(ULONG_MAX);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

hierarchical_mutex high_level_mutex(10000);
hierarchical_mutex low_level_mutex(5000);
hierarchical_mutex other_mutex(6000);

int do_low_level_stuff() { return 5; }

int low_level_func()
{
    std::lock_guard<hierarchical_mutex> lock(low_level_mutex);
    return do_low_level_stuff();
}

void high_level_stuff(int some_param) { std::cout << "woofwoof: " << some_param << '\n'; }

void high_level_func() {
    std::lock_guard<hierarchical_mutex> lock(high_level_mutex);
    high_level_stuff(low_level_func());
}

void thread_a() {
    high_level_func();
}

void do_other_stuff() { std::cout << "miaow\n"; }

void other_stuff() {
    high_level_func();
    do_low_level_stuff();
}

void thread_b() {
    std::lock_guard<hierarchical_mutex> lock(other_mutex);
    other_stuff();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    thread_a();
    
    try {
        thread_b();
    } catch (std::logic_error &e) {
        std::cerr << e.what() << "=> looks like we caught ourselves an exception!\n";
    }
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// woofwoof: 5
// "mutex hierarchy violated"
// => looks like we caught ourselves an exception!
// Program ended with exit code: 0
