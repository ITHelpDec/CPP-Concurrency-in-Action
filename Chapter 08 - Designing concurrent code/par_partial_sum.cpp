#include <future>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

class join_threads {
public:
    join_threads(std::vector<std::thread> &threads) : threads_(threads) { }

    ~join_threads()
    {
        for (auto &t : threads_) {
            if (t.joinable()) { t.join(); }
        }
    }

private:
    std::vector<std::thread> &threads_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace par {
template <typename _InputIterator>
void partial_sum(_InputIterator __first, _InputIterator __last) {
    typedef typename _InputIterator::value_type T;
    
    struct process_chunk {
        void operator() (_InputIterator begin, _InputIterator last, std::future<T> *prev_end_val, std::promise<T> *end_val)
        {
            try {
                _InputIterator end = last;
                ++end;
                
                std::partial_sum(begin, end, begin);
                
                if (prev_end_val) {
                    const T &addend = prev_end_val->get();
                    *last += addend;
                    
                    if (end_val) { end_val->set_value(*last); }
                    
                    std::for_each(begin, last, [addend] (T &item) {
                        item += addend;
                    });
                }
                
                else if (end_val) { end_val->set_value(*last); }
            }
            
            catch (...) {
                if (end_val) {
                    end_val->set_exception(std::current_exception());
                } else {
                    throw;
                }
            }
        }
    };
    
    std::size_t length = std::distance(__first, __last);
    if (!length) { return; }
    
    std::size_t min_per_thread = 25;
    std::size_t max_threads = (length + min_per_thread - 1) / min_per_thread;
    std::size_t hw_threads = std::thread::hardware_concurrency();
    std::size_t num_threads = std::min(hw_threads ? hw_threads : 2, max_threads);
    
    std::size_t block_size = length / num_threads;
    
    typedef typename _InputIterator::value_type T;
    
    std::vector<std::thread> threads(num_threads - 1);
    std::vector<std::promise<T>> end_vals(num_threads - 1);
    
    std::vector<std::future<T>> prev_end_vals;
    prev_end_vals.reserve(num_threads - 1);
    
    join_threads joiner(threads);
    
    _InputIterator block_start = __first;
    
    for (std::size_t i = 0; i != num_threads - 1; ++i) {
        _InputIterator block_last = block_start;
        std::advance(block_last, block_size - 1);
        
        threads[i] = std::thread(process_chunk(), block_start, block_last,
                                 i ? &prev_end_vals[i - 1] : 0, &end_vals[i]);
        
        block_start = block_last;
        ++block_start;
        
        prev_end_vals.push_back(end_vals[i].get_future());
    }
    
    _InputIterator final_element = block_start;
    
    std::advance(final_element, std::distance(block_start, __last) - 1);
    
    process_chunk() (block_start, final_element,
                     num_threads > 1 ? &prev_end_vals.back() : 0, 0);
}
} //namespace par (parallel)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void printVec(const std::vector<int> &ivec) {
    for (int i : ivec) {
        std::cout << i << ' ';
    } std::cout << '\n';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::vector<int> ivec(10);
    std::iota(ivec.begin(), ivec.end(), 0);
    
    std::vector<int> stdoutvec(ivec.begin(), ivec.end());
    std::vector<int> paroutvec(ivec.begin(), ivec.end());
    
    std::cout << "Before...\n";
    std::cout << "ivec:      "; printVec(ivec);
    std::cout << "stdoutvec: "; printVec(stdoutvec);
    std::cout << "paroutvec: "; printVec(paroutvec);
    
    std::partial_sum(stdoutvec.begin(), stdoutvec.end(), stdoutvec.begin());
    par::partial_sum(paroutvec.begin(), paroutvec.end());
    
    std::cout << "\nAfter...\n";
    std::cout << "ivec:      "; printVec(ivec);
    std::cout << "stdoutvec: "; printVec(stdoutvec);
    std::cout << "paroutvec: "; printVec(paroutvec);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Before...
// ivec:      0 1 2 3 4 5 6 7 8 9
// stdoutvec: 0 1 2 3 4 5 6 7 8 9
// paroutvec: 0 1 2 3 4 5 6 7 8 9
//
// After...
// ivec:      0 1 2 3 4 5 6 7 8 9
// stdoutvec: 0 1 3 6 10 15 21 28 36 45
// paroutvec: 0 1 3 6 10 15 21 28 36 45
// Program ended with exit code: 0
