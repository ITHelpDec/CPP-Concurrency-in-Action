#include <iostream>
#include <thread>
#include <vector>
#include <future>

class join_threads {
public:
    explicit join_threads(std::vector<std::thread> &threads) : threads_(threads) { }
    
    ~join_threads()
    {
        for (auto &t : threads_) {
            if (t.joinable()) { t.join(); }
        }
    }
private:
    std::vector<std::thread> &threads_;
};

namespace par {
template <typename _InputIterator, typename _Function>
void for_each(_InputIterator __first, _InputIterator __last, _Function __f) {
    std::size_t length = std::distance(__first, __last);
    if (!length) { return; }
    
    std::size_t min_per_thread = 25;
    std::size_t max_threads    = (length + min_per_thread - 1) / min_per_thread;
    std::size_t hw_threads     = std::thread::hardware_concurrency();
    std::size_t num_threads    = std::min((hw_threads ? hw_threads : 2), max_threads);
    std::size_t block_size     = length / num_threads;
    
    std::vector<std::future<void>> futures(num_threads - 1);
    std::vector<std::thread>       threads(num_threads - 1);
    
    join_threads joiner(threads); // TBC
    
    _InputIterator block_start = __first;
    
    for (std::size_t i = 0; i != num_threads - 1; ++i) {
        _InputIterator block_end = block_start;
        std::advance(block_end, block_size);
        
        std::packaged_task<void(void)> task([=] () {
            std::for_each(block_start, block_end, __f);
        });
        
        futures[i] = task.get_future();
        threads[i] = std::thread(std::move(task));
        
        block_start = block_end;
    }
    
    std::for_each(block_start, __last, __f);
    
    for (auto &f : futures) { f.get(); }
}
} // namespace par (parallel)

template <typename T>
void printVec(const std::vector<T> &tvec) {
    for (auto &&t : tvec) {
        std::cout << t << ' ';
    } std::cout << '\n';
}

int main()
{
    std::vector<int> ivec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    std::cout << "ivec: ";
    printVec(ivec);
    
    par::for_each(ivec.begin(), ivec.end(), [] (int &val) { val *= val; } );
    
    std::cout << "ivec: ";
    printVec(ivec);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ivec: 1 2 3 4 5 6 7 8 9 10
// ivec: 1 4 9 16 25 36 49 64 81 100
// Program ended with exit code: 0
