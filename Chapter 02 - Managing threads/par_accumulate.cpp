/*
#include <iostream>
#include <thread>

class background_task {
public:
    void operator()() const { std::cout << "woof\n"; }
};



int main()
{
    // std::thread my_thread(background_task());
    // std::thread my_thread{background_task()}
    // std::thread my_thread(background_task{});
    
    std::thread my_thread( [] () { std::cout << "woof\n"; } );
    
    my_thread.join();
    
    return 0;
}
*/

/*
#include <iostream>
#include <thread>

struct Func {
    int &i_;
    
    Func(int &i) : i_(i) { }
    
    void operator()();
    void oops();
};

void Func::operator()()
{
    for (std::size_t j = 0; j != 1000000; ++j) { std::cout << "woof!\n"; }
}

void Func::oops()
{
    int some_local_state = 0;
    Func my_func(some_local_state);
    
    std::thread my_thread(my_func);
    my_thread.detach();
}
*/

/*
#include <thread>

struct Dog { void woof(); };
Dog my_dog;
std::thread t(&Dog::woof, &my_dog);
*/

/*
#include <memory>
#include <thread>

void woof(std::unique_ptr<int>);
auto p = std::make_unique<int>(42);
std::thread t(woof, std::move(p));
*/

/*
#include <thread>
#include <iostream>

void woof() { }
void miaow() { }

int main()
{
    std::thread t1(woof);           // move from temporaries is implicit
    std::thread t2 = std::move(t1); // ownership transferred
    t1 = std::thread(miaow);
    std::thread t3;
    t3 = std::move(t2);
    t1 = std::move(t2);
    
    std::cout << "t1: " << t1.joinable() << '\n';
    std::cout << "t2: " << t2.joinable() << '\n';
    std::cout << "t3: " << t3.joinable() << '\n';
    
    return 0;
}
*/

/*
#include <thread>

void some_function() { }
void some_other_function() { }

int main()
{
    std::thread t1(some_function);
    std::thread t2=std::move(t1);
    t1=std::thread(some_other_function);
    std::thread t3;
    t3=std::move(t2);
    t1=std::move(t3);
    
    return 0;
}
*/

/*
#include <thread>
#include <iostream>

class jthread {
public:
    jthread() noexcept = default;
    
    template <typename Callable, typename ...Args>
    explicit jthread(Callable &&func, Args &&...rest) : t_(std::forward<Callable>(func), std::forward<Args>(rest)...) { }
    
    explicit jthread(std::thread t) noexcept : t_(std::move(t)) { }
    jthread& operator=(std::thread other) noexcept
    {
        if (joinable()) { join(); }
        t_ = std::move(other);
        return *this;
    }
    
    jthread(jthread &&other) noexcept : t_(std::move(other.t_)) { }
    jthread& operator=(jthread &&other) noexcept
    {   // this will only work if you build a joinable() member function...
        if (joinable()) { join(); }
        t_ = std::move(other.t_);
        return *this;
    }
    
    ~jthread() noexcept
    {
        if (joinable()) { join(); }
    }
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::thread::id get_id() const noexcept { return t_.get_id(); }
    
    bool joinable() const noexcept { return t_.joinable(); }
    
    void join() { t_.join(); }
    void detach() { t_.detach(); }
    void swap(jthread &other) noexcept { t_.swap(other.t_); }
    
    std::thread& as_thread() noexcept { return t_; }
    const std::thread& as_thread() const noexcept { return t_; }
    
private:
    std::thread t_;
};

void woof() { std::cout << "woof\n"; }

void test(bool flag, int two, double pi) { std::cout << flag << ' ' << two << ' ' << pi << '\n'; }

int main()
{
    jthread t(woof);
    
    jthread t2(test, true, 2, 3.14);
    
    return 0;
}
*/

/*
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
*/

#include <benchmark/benchmark.h>

#include <numeric>
#include <thread>
#include <iostream>
#include <vector>

template <typename Iterator, typename T>
struct accumulate_block {
    void operator() (Iterator first, Iterator last, T &result)
    {
        result = std::accumulate(first, last, result);
    }
};

namespace par {
template <typename Iterator, typename T>
T accumulate(Iterator first, Iterator last, T init) {
    const unsigned long length = std::distance(first, last);
    
    if (!length) { return init; }
    
    const unsigned long min_per_thread = 25;
    const unsigned long max_threads = (length + min_per_thread - 1) / min_per_thread;
    const unsigned long hardware_threads = std::thread::hardware_concurrency();
    const unsigned long num_threads = std::min(hardware_threads ? hardware_threads : 2, max_threads);
    const unsigned long block_size = length / num_threads;
    
    std::vector<T> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);
    
    Iterator block_start = first;
    
    for (unsigned long i = 0; i < num_threads - 1; ++i) {
        Iterator block_end = block_start;
        std::advance(block_end, block_size);
        threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
        block_start = block_end;
    }
    
    accumulate_block<Iterator, T>() (block_start, last, results[num_threads - 1]);
    
    for (auto &thread : threads) { thread.join(); }
    
    return std::accumulate(results.begin(), results.end(), init);
}

}

std::vector<int> ivec = { 1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
    1, 2, 3, 4 ,5 ,6, 7, 8, 9, 1, 2, 3, 4, 5 ,6, 7, 8, 9, 4, 6, 1, 7, 9, 2, 8,
};

static void bm_std_acc(benchmark::State &state) {
    for (auto _ : state) {
        auto res = std::accumulate(ivec.begin(), ivec.end(), 0);
        benchmark::DoNotOptimize(res);
    }
} BENCHMARK(bm_std_acc);

static void bm_par_acc(benchmark::State &state) {
    for (auto _ : state) {
        auto res = par::accumulate(ivec.begin(), ivec.end(), 0);
        benchmark::DoNotOptimize(res);
    }
} BENCHMARK(bm_par_acc);

static void bm_std_red(benchmark::State &state) {
    for (auto _ : state) {
        auto res = std::reduce(ivec.begin(), ivec.end(), 0);
        benchmark::DoNotOptimize(res);
    }
} BENCHMARK(bm_std_red);

BENCHMARK_MAIN();

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Run on (12 X 24.1205 MHz CPU s)
// CPU Caches:
//   L1 Data 64 KiB
//   L1 Instruction 128 KiB
//   L2 Unified 4096 KiB (x12)
// Load Average: 1.24, 1.17, 1.16
// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bm_std_acc       3460 ns         3460 ns       202281
// bm_par_acc      87506 ns        86746 ns         7302
// bm_std_red       3908 ns         3908 ns       179985
// Program ended with exit code: 0
