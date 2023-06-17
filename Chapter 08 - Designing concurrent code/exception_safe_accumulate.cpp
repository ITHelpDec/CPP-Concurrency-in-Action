#include <vector>
#include <thread>
#include <numeric>
#include <iostream>
#include <future>

class join_threads {
public:
    explicit join_threads(std::vector<std::thread> &threads) : threads_(threads) { }

    ~join_threads()
    {
        for (auto &t : threads_)
            if (t.joinable()) { t.join(); }
    }
private:
    std::vector<std::thread> &threads_;
};

template <typename __ForwardIt, typename __Tp>
struct accumulate_block {
    __Tp operator() (__ForwardIt __first, __ForwardIt __last)
    {
        return std::accumulate(__first, __last, __Tp());
    }
};

namespace par {
template <typename __ForwardIt, typename __Tp>
__Tp accumulate(__ForwardIt __first, __ForwardIt __last, __Tp __init) {
    std::size_t length = std::distance(__first, __last);
    if (!length) { return __init; }
    
    std::size_t min_per_thread = 25;
    std::size_t max_threads    = (length + min_per_thread - 1) / min_per_thread;
    std::size_t hw_threads     = std::thread::hardware_concurrency();
    std::size_t num_threads    = std::min((hw_threads ? hw_threads : 2), max_threads);
    std::size_t block_size     = length / num_threads;
    
    std::vector<std::future<__Tp>> futures(num_threads - 1);
    std::vector<std::thread> threads(num_threads - 1);
    
    join_threads joiner(threads);
    
    __ForwardIt block_start = __first;
    
    for (int i = 0; i != num_threads - 1; ++i) {
        __ForwardIt block_end = block_start;
        std::advance(block_start, block_size);
        
        std::packaged_task<__Tp(__ForwardIt, __ForwardIt)> task((accumulate_block<__ForwardIt, __Tp>()));
        
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task), block_start, block_end);
        
        block_start = block_end;
    }
    
    __Tp last_result = accumulate_block<__ForwardIt, __Tp>()(block_start, __last);
    
    __Tp result = __init;
    
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
