#include <numeric>
#include <thread>
#include <iostream>

template <typename __ForwardIt, typename __Tp>
struct accumulate_block {
    void operator() (__ForwardIt first, __ForwardIt last, __Tp &result)
    {
        result = std::accumulate(first, last, result);
    }
};

namespace par {
template <typename __ForwardIt, typename __Tp>
__Tp accumulate(__ForwardIt first, __ForwardIt last, __Tp init) {
    std::size_t length = std::distance(first, last);
    
    if (!length) { return init; }
    
    std::size_t min_per_thread = 25;
    std::size_t max_threads = (length + min_per_thread - 1) / min_per_thread;
    std::size_t hw_threads = std::thread::hardware_concurrency();
    std::size_t num_threads = std::min((hw_threads ? hw_threads : 2), max_threads);
    std::size_t block_sz = length / num_threads;
    
    std::vector<__Tp> results(num_threads);
    std::vector<std::thread> threads(num_threads - 1);
    
    __ForwardIt block_start = first;
    
    for (int i = 0; i != num_threads - 1; ++i) {
        __ForwardIt block_end = block_start;
        std::advance(block_end, block_sz);
        
        // threads[i] = std::thread(accumulate_block<__ForwardIt, __Tp>(), block_start, block_end, std::ref(results[i]));
        threads[i] = std::thread([&, i] () {
            accumulate_block<__ForwardIt, __Tp>()(block_start, block_end, results[i]);
        });
        
        block_start = block_end;
    }
    
    accumulate_block<__ForwardIt, __Tp>()(block_start, last, results[num_threads - 1]);
    
    // std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    // why not just use range-based for loop?
    for (auto &t : threads) { t.join(); }
    
    return std::accumulate(results.begin(), results.end(), init);
}
} // namespace par (parallel)

int main()
{
    std::vector<int> ivec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    int result = par::accumulate(ivec.begin(), ivec.end(), 0);
    
    std::cout << "result: " << result << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// result: 55
// Program ended with exit code: 0
