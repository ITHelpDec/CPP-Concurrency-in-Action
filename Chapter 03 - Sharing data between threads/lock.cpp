#include <iostream>
#include <thread>

struct some_big_object {
    int thing_;
    some_big_object(int x) : thing_(x) { };
};

void swap(some_big_object &lhs, some_big_object &rhs) {
    using std::swap;
    swap(lhs.thing_, rhs.thing_);
}

class X {
    friend void swap(some_big_object&, some_big_object&);
public:
    X(const some_big_object &some_detail) : some_detail_(some_detail) { }
    
    // bit of a weird way to write a swap function
    // would we not be better off specifying "other" instead of "lhs" and "rhs"?
    void swap(X &lhs, X &rhs)
    {
        if (&lhs == &rhs) { return; }
        std::lock(lhs.m, rhs.m);
        std::lock_guard<std::mutex> lock1(lhs.m, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(rhs.m, std::adopt_lock);
        ::swap(lhs.some_detail_, rhs.some_detail_);
    }
    
    int print() { return some_detail_.thing_; }
    
private:
    some_big_object some_detail_;
    std::mutex m;
};

int main()
{
    X lhs(some_big_object(6)), rhs(some_big_object(9));
    std::cout << "lhs: " << lhs.print() << ", rhs: " << rhs.print() << '\n';
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    lhs.swap(lhs, rhs);
    std::cout << "lhs: " << lhs.print() << ", rhs: " << rhs.print() << '\n';
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::thread t1(&X::swap, &lhs, std::ref(lhs), std::ref(rhs));
    std::thread t2(&X::swap, &rhs, std::ref(lhs), std::ref(rhs));
    
    std::cout << "lhs: " << lhs.print() << ", rhs: " << rhs.print() << '\n';
    std::cout << "lhs: " << lhs.print() << ", rhs: " << rhs.print() << '\n';
    
    t1.join();
    t2.join();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    rhs.swap(lhs, rhs);
    std::cout << "lhs: " << lhs.print() << ", rhs: " << rhs.print() << '\n';

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// lhs: 6, rhs: 9
// lhs: 9, rhs: 6
// lhs: 9, rhs: 6
// lhs: 9, rhs: 6
// lhs: 6, rhs: 9
// Program ended with exit code: 0
