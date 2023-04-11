#include <string>
#include <future>
#include <iostream>

struct X {
    // author provides incomplete functions yet again....
    void foo(int i, const std::string &s)
    {
        std::cout << s << ", " << i << std::endl;
    }
    
    std::string bar(const std::string &s)
    {
        return s + ", cruel world!";
    }
};

struct Y {
    double operator() (double d) { return d * 2; }
};

// incomplete function
X baz(X &x) { return X(); }

class move_only {
public:
    move_only() = default;
    
    move_only(const move_only &other) = default;
    move_only(move_only &&m) = default;
    
    move_only& operator=(const move_only &m) = default;
    move_only& operator=(move_only &&m) = default;
    
    void operator()() { std::cout << "miaow\n"; }
};

int main()
{
    X x;
    
    auto f1 = std::async(&X::foo, &x, 42, "hello"); // ref to x
    auto f2 = std::async(&X::bar, x, "goodbye");    // copy of x
    
    Y y;
    
    auto f3 = std::async(Y(), 3.142);           // move constructed
    auto f4 = std::async(std::ref(y), 2.718);   // calls y(2.718)
    
    std::async(baz, std::ref(x));   // calls baz(x)
    
    auto f5 = std::async(move_only());  // temp-constructed from std::move(move_only())
    
    auto f6 = std::async(std::launch::async, Y(), 1.2); // new thread of Y()
    
    // run when .wait() or .get() is called
    auto f7 = std::async(std::launch::deferred, baz, std::ref(x));
    
    auto f8 = std::async( // implementation chooses when launched
                         std::launch::async | std::launch::deferred,
                         baz, std::ref(x));
    
    // as f8 (std::launch not declared)
    auto f9 = std::async(baz, std::ref(x));
    
    f7.wait();  // invoke deferred function
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// hello, 42
// miaow
// Program ended with exit code: 0
