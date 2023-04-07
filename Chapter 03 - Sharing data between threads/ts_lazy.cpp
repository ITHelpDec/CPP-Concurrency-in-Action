#include <memory>
#include <mutex>
#include <iostream>

std::shared_ptr<int> shared_int;
std::mutex init_mutex;

void init() {
    std::unique_lock<std::mutex> lock(init_mutex);
    
    if (!shared_int) {
        // shared_int.reset(new int(100));
        
        // worth opting for assignment over raw `new` call for efficiency and exception safety reasons
        // EDIT: in hindsight, it seems there may be subtle differences between .reset() and operator=
        // https://stackoverflow.com/a/31439356 for more info
        shared_int = std::make_shared<int>(100);
    }
    
    lock.unlock();
}

int main()
{
    std::cout << "before init(): " << shared_int.get() << " (uninitialised)\n";
    
    init();
    
    std::cout << "after init():  " << shared_int.get() << " (" << *shared_int.get() << ")\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// before init(): 0x0 (uninitialised)
// after init():  0x6000002054f8 (100)
// Program ended with exit code: 0
