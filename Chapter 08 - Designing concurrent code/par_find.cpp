#include <future>
#include <iostream>
#include <thread>
#include <numeric>
#include <vector>
#include <random>
#include <chrono>

int limit = 100000;

int num() {
    static std::default_random_engine e(std::random_device{}());
    static std::uniform_int_distribution u(0, limit);
    return u(e);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class join_threads {
public:
    explicit join_threads(std::vector<std::thread> &threads) : threads_(threads) { }
    
    ~join_threads()
    {
        for (auto &t :threads_) {
            if (t.joinable()) { t.join(); }
        }
    }
private:
    std::vector<std::thread> &threads_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace par {
template <typename _InputIterator, typename _Tp>
_InputIterator find(_InputIterator __first, _InputIterator __last, _Tp __value) {
    struct find_element {
        void operator() (_InputIterator __begin, _InputIterator __end, _Tp __match, std::promise<_InputIterator> *__result, std::atomic<bool> *__done)
        {
            try {
                for ( ; __begin != __end && !__done->load(); ++__begin) {
                    if (*__begin == __match) {
                        __result->set_value(__begin);
                        __done->store(true);
                        return;
                    }
                }
            } catch (...) {
                try {
                    __result->set_exception(std::current_exception());
                    __done->store(true);
                } catch (...) {
                    // do nada
                }
            }
        }
    };
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::size_t length = std::distance(__first, __last);
    if (!length) { return __last; }
    
    // thread size boilerplate
    std::size_t min_per_thread = 25;
    std::size_t max_threads = (length + min_per_thread - 1) / min_per_thread;
    std::size_t hw_threads = std::thread::hardware_concurrency();
    std::size_t num_threads = std::min(hw_threads ? hw_threads : 2, max_threads);
    
    std::size_t block_size = length / num_threads;
    
    std::promise<_InputIterator> result;
    std::atomic<bool> done(false);
    
    std::vector<std::thread> threads(num_threads - 1);
    
    {
        join_threads joinder(threads);
        
        _InputIterator block_start = __first;
        
        // we only modify threads, so chossing to opt for range-based
        // for (unsigned long i = 0; i < num_threads - 1; ++i)
        
        for (auto &t : threads) {
            _InputIterator block_end = block_start;
            std::advance(block_end, block_size);
            
            t = std::thread(find_element(), block_start, block_end, __value, &result, &done);
            
            block_start = block_end;
        }
        
        find_element()(block_start, __last, __value, &result, &done);
    }
    
    if (!done.load()) { return __last; }
    
    return result.get_future().get();
    
}
} // namespace par (parallel)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::vector<int> ivec(limit);
    std::iota(ivec.begin(), ivec.end(), 0);
    
    int val = num();
    
    auto t1 = std::chrono::high_resolution_clock::now();
    
    auto a = std::find(ivec.begin(), ivec.end(), val);
    std::cout << "std::find: " << val << " found at index " << std::distance(ivec.begin(), a) << '\n';
    
    auto t2 = std::chrono::high_resolution_clock::now();
    auto result1 = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    auto t3 = std::chrono::high_resolution_clock::now();
    
    auto b = par::find(ivec.begin(), ivec.end(), val);
    std::cout << "par::find: " << val << " found at index " << std::distance(ivec.begin(), b) << '\n';
    
    auto t4 = std::chrono::high_resolution_clock::now();
    auto result2 = std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3).count();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    auto t5 = std::chrono::high_resolution_clock::now();
    
    auto c = par::find(ivec.begin(), ivec.end(), val);
    std::cout << "par::find: " << val << " found at index " << std::distance(ivec.begin(), a) << '\n';
    
    auto t6 = std::chrono::high_resolution_clock::now();
    auto result3 = std::chrono::duration_cast<std::chrono::nanoseconds>(t6 - t5).count();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    auto t7 = std::chrono::high_resolution_clock::now();
    
    auto d = std::find(ivec.begin(), ivec.end(), val);
    std::cout << "std::find: " << val << " found at index " << std::distance(ivec.begin(), b) << '\n';
    
    auto t8 = std::chrono::high_resolution_clock::now();
    auto result4 = std::chrono::duration_cast<std::chrono::nanoseconds>(t8 - t7).count();
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "\nstd::find took " << result1 << "ns\n";
    std::cout << "par::find took " << result2 << "ns\n";
    std::cout << "par::find took " << result3 << "ns\n";
    std::cout << "std::find took " << result4 << "ns\n\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// std::find: 58213 found at index 58213
// par::find: 58213 found at index 58213
// par::find: 58213 found at index 58213
// std::find: 58213 found at index 58213
//
// std::find took 4217958ns
// par::find took 3417042ns
// par::find took 72923667ns
// std::find took 4854125ns
//
// Program ended with exit code: 0
