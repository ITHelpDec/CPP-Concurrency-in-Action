#include <iostream>
#include <thread>

void hello() {
    std::cout << "Hello, concurrent world!\n";
}

int main()
{
    std::thread t(hello);
    t.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Hello, concurrent world!
// Program ended with exit code: 0
