#include <vector>
#include <thread>
#include <iostream>

void do_work(unsigned id) { std::cout << id << ' '; }

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

// 1 2 06 48  3 7  9 5 10 11 12 13 14 15 16 17 18 19
// Program ended with exit code: 0
