#include <deque>
#include <iostream>

namespace std {

template <class T, class Container = std::deque<T>>
class queue {
public:
    // all constructors are default

    queue() = default;
    ~queue() = default;

    queue(const queue&) = default;
    queue(queue&&) = default;

    queue& operator=(const queue&) = default;
    queue& operator=(queue&&) = default;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    explicit queue(const Container &c) : c_(c) { }
    explicit queue(Container &&c) noexcept : c_(std::move(c)) { }

    template <class Alloc> explicit queue(const Alloc &a) : c_(a) { }
    template <class Alloc> queue(const Container &c, const Alloc &a) : c_(c, a) { }
    template <class Alloc> queue(Container &&c, const Alloc &a) noexcept
        : c_(std::move(c), a) { }

    //  author left out const queue& - - - - - - - - - - - - - - - - - - - - - -
    template <class Alloc> queue(const queue &q, const Alloc &a) : c_(q.c_, a) { }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    template <class Alloc> queue(queue &&q, const Alloc &a) noexcept
        : c_(std::move(q.c_), a) { }

    bool empty() const { return c_.empty(); }
    std::size_t size() const { return c_.size(); }

    T& front() { return c_.front(); }
    const T& front() const { return c_.front(); }

    T& back() { return c_.back(); }
    const T& back() const { return c_.back(); }

    void push(const T &t) { c_.push_back(t); }
    void push(T &&t) noexcept { c_.push_back(std::move(t)); }

    void pop() { c_.pop_front(); }
    void swap(queue &q) noexcept
    {
        using std::swap;
        swap(c_, q.c_);
    }

    template <class... Args> void emplace(Args &&...rest )
    {
        c_.emplace_back(std::forward<Args>(rest)...);
    }
    
protected:
    Container c_;
};

} // namespace std;

int main()
{
    std::cout << std::boolalpha;
    
    std::cout << "Instatiating queue...\n";
    std::queue<int> iq;
    
    std::cout << "Is our queue empty?: " << iq.empty() << '\n';
    
    std::cout << "Adding an rvalue to our queue...\n";
    iq.push(5);
    
    std::cout << "What is at the front of our queue? " << iq.front() << '\n';
    
    std::cout << "Adding an lvalue to our queue...\n";
    int val = 6;
    iq.push(val);
    
    std::cout << "What is at the back of our queue? " << iq.back() << '\n';
    
    std::cout << "Emplacing an rvalue and lvalue to our queue...\n";
    iq.emplace(7);
    iq.emplace(val + 2);
    
    std::cout << "How many elements do we have in our queue? " << iq.size() << '\n';
    
    std::cout << "Changing the front and back values to \"0\"...\n";
    iq.front() = iq.back() = 0;
    
    std::cout << "iq.front() = \"" << iq.front() << "\"; iq.back() = \"" << iq.back() << "\"\n";
    
    std::cout << "Swapping our queue with another queue...\n";

    std::queue<int> blank;
    
    std::cout << "before: iq.size() = " << iq.size() << "; blank.size() = " << blank.size() << '\n';
    
    iq.swap(blank);
    
    std::cout << "after:  iq.size() = " << iq.size() << "; blank.size() = " << blank.size() << '\n';
    
    std::cout << std::noboolalpha;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Instatiating queue...
// Is our queue empty?: true
// Adding an rvalue to our queue...
// What is at the front of our queue? 5
// Adding an lvalue to our queue...
// What is at the back of our queue? 6
// Emplacing an rvalue and lvalue to our queue...
// How many elements do we have in our queue? 4
// Changing the front and back values to "0"...
// iq.front() = "0"; iq.back() = "0"
// Swapping our queue with another queue...
// before: iq.size() = 4; blank.size() = 0
// after:  iq.size() = 0; blank.size() = 4
// Program ended with exit code: 0
