#include <numeric>
#include <iostream>
#include <thread>
#include <vector>
#include <future>

template <typename __ForwardIt, typename __Tp>
struct accumulate_block {
    __Tp operator() (__ForwardIt first, __ForwardIt last)
    {
        return std::accumulate(first, last, __Tp{});
    }
};

namespace par {
template <typename __ForwardIt, typename __Tp>
__Tp accumulate(__ForwardIt first, __ForwardIt last, __Tp &&init) {
    std::size_t length = std::distance(first, last);
    if (!length) { return init; }
    
    std::size_t min_per_thread   = 25;
    std::size_t max_threads      = (length + min_per_thread - 1) / min_per_thread;
    std::size_t hardware_threads = std::thread::hardware_concurrency();
    std::size_t num_threads      = std::min((hardware_threads ? hardware_threads : 2), max_threads);
    std::size_t block_size       = length / num_threads;
    
    std::vector<std::future<__Tp>> futures(num_threads - 1);
    std::vector<std::thread>       threads(num_threads - 1);
    
    __ForwardIt block_start = first;
    
    for (int i = 0; i != num_threads - 1; ++i) {
        __ForwardIt block_end = block_start;
        std::advance(block_end, block_size);
        
        // "Parentheses were disambiguated as a function declaration"
        // "Add a pair of parentheses to declare a variable"
        // std::packaged_task<__Tp(__ForwardIt, __ForwardIt)> task(accumulate_block<__ForwardIt, __Tp>());
        std::packaged_task<__Tp(__ForwardIt,__ForwardIt)> task((accumulate_block<__ForwardIt, __Tp>()));
        
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task), block_start, block_end);
        
        block_start = block_end;
    }

    // __Tp last_result = accumulate_block<__ForwardIt, __Tp>(block_start, last);
    __Tp last_result = accumulate_block<__ForwardIt, __Tp>()(block_start, last);
    
    // std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    for (auto &t : threads) { t.join(); }
    
    __Tp result = init;
    
    // for (int i = 0; i != num_threads - 1; ++i) { result += futures[i].get(); }
    for (auto &f : futures) { result += f.get(); }
    
    result += last_result;
    
    return result;
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
