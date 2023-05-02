#include <atomic>
#include <iostream>
#include <thread>

class spinlock_mutex {
public:
    void lock()
    {
        while(fleg.test_and_set(std::memory_order_acquire)) {
            // you spin me right round, baby, right round
        }
    }
    
    void unlock()
    {
        fleg.clear(std::memory_order_release);
    }
    
private:
    std::atomic_flag fleg;// = ATOMIC_FLAG_INIT;
};

spinlock_mutex m;

void spin() {
    m.lock();
    
    std::cout << std::this_thread::get_id() << " sleeping...\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << std::this_thread::get_id() << " has slept :)\n";
    
    m.unlock();
}

int main()
{
    std::thread t1(spin);
    std::thread t2(spin);
    
    t1.join();
    t2.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// 0x700004d22000 sleeping...
// 0x700004d22000 has slept :)
// 0x700004da5000 sleeping...
// 0x700004da5000 has slept :)
// Program ended with exit code: 0
