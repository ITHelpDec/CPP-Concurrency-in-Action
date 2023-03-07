#include <vector>
#include <thread>
#include <iostream>

void do_work(unsigned id) { std::cout << id << '\n'; }
void f() {
    std::vector<std::thread> tvec;
    
    for (unsigned i = 0; i < 20; ++i) { tvec.emplace_back(do_work, i); }
    
    for (auto &thread : tvec) { thread.join(); }
}

int main()
{
    f();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

