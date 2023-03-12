#include <list>
#include <mutex>
#include <algorithm>
#include <random>
#include <iostream>

std::list<int> ilist;
std::mutex mtx;

int num() {
    static thread_local std::default_random_engine e(std::random_device{}());
    static std::uniform_int_distribution u(0, 10);
    return u(e);
}

void add(int val) {
    // std::lock_guard<std::mutex> guard(mtx);
    std::lock_guard guard(mtx); // C++17-ism
    ilist.emplace_back(val);
}

bool contains(int val) {
    // std::lock_guard guard(mtx);
    std::scoped_lock guard(mtx); // introduced in C++17
    return std::find(ilist.begin(), ilist.end(), val) != ilist.end();
}

int main()
{
    while (!contains(5)) { add(num()); }
    std::cout << "Found 5!\n";
    
    return 0;
}
