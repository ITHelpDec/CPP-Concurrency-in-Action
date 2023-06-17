#include <iostream>
#include <numeric>
#include <future>

namespace par {
template <typename __ForwardIt, typename __Tp>
__Tp accumulate(__ForwardIt __first, __ForwardIt __last, __Tp init) {
    std::size_t length       = std::distance(__first, __last);
    std::size_t max_chunk_sz = 25;
    
    if (length <= max_chunk_sz) {
        return std::accumulate(__first, __last, init);
    } else {
        __ForwardIt mid_point = __first;
        std::advance(mid_point, length >>= 1);
        
        // std::async(parallel_accumulate<Iterator,T>, first,mid_point,init);
        // no supposed race conditions when passing by either ref or value...?
        auto first_half_result = std::async(std::launch::async, [=] () {
            return par::accumulate(__first, mid_point, init);
        });
        
        __Tp second_half_result = par::accumulate(mid_point, __last, __Tp());
        
        return first_half_result.get() + second_half_result;
    }
}
} // namespace par (parallel)

int main()
{
    std::vector<int> ivec//(10000, 987654);
    = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
        31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
        71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
        81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
        91 ,92, 93, 94, 95, 96, 97, 98, 99, 100
    };
    
    int result = par::accumulate(ivec.begin(), ivec.end(), 20);
    std::cout << "result:  " << result << '\n';
    
    int result2 = std::accumulate(ivec.begin(), ivec.end(), 20);
    std::cout << "result2: " << result2 << '\n';
    
    int result3 = std::reduce(ivec.begin(), ivec.end(), 20);
    std::cout << "result3: " << result3 << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// result:  5070
// result2: 5070
// result3: 5070
// Program ended with exit code: 0
