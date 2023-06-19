#include <vector>
#include <thread>
#include <iostream>
#include <future>
#include <numeric>

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
    
    if (length < (min_per_thread * 2)) {
        std::for_each(__first, __last, __f);
    } else {
        _InputIterator mid_point = __first + (length >>= 1);
        
        std::future<void> first_half = std::async(&par::for_each<_InputIterator, _Function>, __first, mid_point, __f);
        
        par::for_each(mid_point, __last, __f);
        
        first_half.get();
    }
}
} // namespace par (parallel)

template <typename T>
void printVec(const std::vector<T> &tvec) {
    for (auto &&t : tvec) {
        std::cout << t << ' ';
    } std::cout << '\n';
}

template <typename T>
void double_elements(std::vector<T> &tvec) {
    par::for_each(tvec.begin(), tvec.end(), [] (T &val) { val <<= 2; } );
}

int main()
{
    std::vector<int> ivec(100);
    std::iota(ivec.begin(), ivec.end(), 0);
    
    std::cout << "ivec: ";
    printVec(ivec);
    
    double_elements(ivec);
    
    std::cout << "ivec: ";
    printVec(ivec);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ivec: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99
// ivec: 0 4 8 12 16 20 24 28 32 36 40 44 48 52 56 60 64 68 72 76 80 84 88 92 96 100 104 108 112 116 120 124 128 132 136 140 144 148 152 156 160 164 168 172 176 180 184 188 192 196 200 204 208 212 216 220 224 228 232 236 240 244 248 252 256 260 264 268 272 276 280 284 288 292 296 300 304 308 312 316 320 324 328 332 336 340 344 348 352 356 360 364 368 372 376 380 384 388 392 396
// Program ended with exit code: 0
