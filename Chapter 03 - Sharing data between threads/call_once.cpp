#include <memory>
#include <mutex>
#include <iostream>

std::shared_ptr<int> shared_int;
std::once_flag int_flag;

// void init()
// {
//     shared_int = std::make_shared<int>(100);
// }

void init()
{
    // can also use a lambda instead of defining another function
    std::call_once(int_flag, [] () { shared_int = std::make_shared<int>(100); } );
}

int main()
{
    std::cout << "before init(): " << shared_int.get() << " (= nullptr)\n";
    
    init();
    
    std::cout << "after  init(): " << shared_int.get() << " (= " << *shared_int.get() << ")\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// before init(): 0x0 (= nullptr)
// after  init(): 0x6000002054f8 (= 100)
// Program ended with exit code: 0
