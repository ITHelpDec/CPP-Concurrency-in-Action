#include <future>
#include <iostream>

// it's worth noting that std::result_of(_t) is seen as deprecated in newer compilers
// as of making this commmit, it is currently advisable to opt for std::inoke_result(_t)

// #1
// template <typename Func, typename T>
// std::future<typename std::result_of<Func(Args&&)>::type> spawn_task(Func &&f, T &&t) {

// #2
// template <typename Func, typename T>
// std::future<std::result_of_t<Func(T&&)>> spawn_task(Func &&f, T &&t) {

// #3
// template <typename Func, typename ...Args>
// std::future<typename std::invoke_result<Func&&, Args&&...)>::type> spawn_task(Func &&f, Args &&...rest) {

// #4
template <typename Func, typename ...Args>
std::future<std::invoke_result_t<Func&&, Args&&...>> spawn_task(Func &&f, Args &&...rest) {
    // local typedef for improved legibility
    typedef std::invoke_result_t<Func&&, Args&&...> result_type;

    // create a packaged task for our function
    std::packaged_task<result_type(Args&&...)> task(std::move(f));

    // create a future to capture the result of our task
    std::future<result_type> result(task.get_future());

    // create a worker thread to go off with our task and arguments
    std::thread worker(std::move(task), std::move(rest)...);
    worker.detach();

    // when ready...
    return result;
}

int links(int x, int y, int z) {
    return x + y + z;
}

int main()
{
    auto fut = spawn_task(links, 2, 3, 4);
    
    std::cout << "future test result: " << fut.get() << '\n';
    
    return 0;
}
