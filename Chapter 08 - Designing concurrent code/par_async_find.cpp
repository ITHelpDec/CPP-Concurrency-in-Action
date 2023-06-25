#include <atomic>
#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <random>

int limit = 10000;

int num() {
    static std::default_random_engine e(std::random_device{}());
    static std::uniform_int_distribution u(0, limit);
    return u(e);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace par {

// helper function
template <typename _InputIterator, typename _Tp>
_InputIterator find(_InputIterator __first, _InputIterator __last, _Tp __value, std::atomic<bool> &done) {
    try {
        std::size_t length = std::distance(__first, __last);
        std::size_t min_per_thread = 25;
        
        if (length < (min_per_thread * 2)) {
            for ( ; __first != __last && !done.load(); ++__first) {
                if (*__first == __value) {
                    done.store(true);
                    return __first;
                }
            }
            
            return __last;
        } else {
            _InputIterator mid_point = __first + (length >> 1);
            
            std::future<_InputIterator> async_result = std::async(&par::find<_InputIterator, _Tp>, mid_point, __last, __value, std::ref(done));
            
            _InputIterator direct_result = par::find(__first, mid_point, __value, done);
            
            return direct_result == mid_point ? async_result.get() : direct_result;
        }
    } catch (...) {
        done.store(true);
        std::cout << "No dice :(\n";
        throw;
    }
}

// main function
template <typename _InputIterator, typename _Tp>
_InputIterator find(_InputIterator __first, _InputIterator __last, _Tp __value) {
    std::atomic<bool> done;
    return par::find(__first, __last, __value, done);
}
} // namespace par (parallel)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::vector<int> ivec(limit);
    std::iota(ivec.begin(), ivec.end(), 0);
    
    int val = num();
    auto a = std::find(ivec.begin(), ivec.end(), val);
    std::cout << "std::find: " << val << " found at index " << std::distance(ivec.begin(), a) << '\n';
    
    auto b = par::find(ivec.begin(), ivec.end(), val);
    std::cout << "par::find: " << val << " found at index " << std::distance(ivec.begin(), b) << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// std::find: 3039 found at index 3039
// par::find: 3039 found at index 3039
// Program ended with exit code: 0
