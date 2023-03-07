#include <benchmark/benchmark.h>

#include <numeric>
#include <thread>
#include <iostream>
#include <vector>
#include <random>

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int num() {
    static std::default_random_engine e;
    static std::uniform_int_distribution u(0, 100);
    return u(e);
}

std::vector<int> ivec(10000, num());

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

// Run on (12 X 24.0119 MHz CPU s)
// CPU Caches:
//   L1 Data 64 KiB
//   L1 Instruction 128 KiB
//   L2 Unified 4096 KiB (x12)
// Load Average: 1.14, 1.19, 1.16
// -----------------------------------------------------
// Benchmark           Time             CPU   Iterations
// -----------------------------------------------------
// bm_std_acc      41887 ns        41887 ns        12710
// bm_par_acc      92763 ns        90698 ns         8142
// bm_std_red      47494 ns        47494 ns        14696
// Program ended with exit code: 0
