#include <future>
#include <thread>
#include <iostream>

template <typename Func, typename ...Args>
std::future<decltype(std::declval<Func>()())> spawn_async(Func &&func) {
    std::promise<decltype(std::declval<Func>()())> p;
    
    auto result = p.get_future();
    
    std::thread t([p = std::move(p), f = std::decay_t<Func>(f)] () mutable {
        try {
            p.set_value_at_thread_exit(f());
        } catch (...) {
            p.set_exception_at_thread_exit(std::current_exception());
        }
    });
    
    t.detach();
    
    return result;
}

void test() {
    std::cout << "testing spawn async\n";
}

int main()
{
    // does not compile
    auto fut1 = spawn_async([] () {} );
    
    // doesn't compile either
    auto fut2 = spawn_async(test());
    
    return 0;
}
