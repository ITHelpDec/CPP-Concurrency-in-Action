#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <numeric>

class barrier {
public:
    barrier(std::size_t count) : count_(count), spaces_(count), generation_(0) { }

    void wait()
    {
        std::size_t my_gen = generation_.load();

        if (!--spaces_) {
            spaces_ = count_.load();
            ++generation_;
        } else {
            while (generation_.load() == my_gen) { std::this_thread::yield(); }
        }
    }

    void done_waiting()
    {
        --count_;

        if (!--spaces_) {
            spaces_ = count_.load();
            ++generation_;
        }
    }

private:
    std::atomic<std::size_t> count_;

    std::atomic<std::size_t> spaces_;
    std::atomic<std::size_t> generation_;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

namespace par {
template <typename _Iterator>
void partial_sum(_Iterator __first, _Iterator __last) {
    typedef typename _Iterator::value_type T;

    struct process_element {
        void operator() (_Iterator __first, _Iterator __last, std::vector<T> &buffer, std::size_t i, barrier &b)
        {
            T &ith_element = *(__first + i);
            bool update_source = false;

            for (std::size_t step = 0, stride = 1; stride <= i; ++step, stride *= 2) {
                T &source = step % 2 ? buffer[i]          : ith_element;
                T &dest   = step % 2 ? ith_element        : buffer[i];
                T &addend = step % 2 ? buffer[i - stride] : *(__first + i - stride);

                dest = source + addend;
                update_source = !(step % 2);
                b.wait();
            }

            if (update_source) {
                ith_element = buffer[i];
            } else {
                buffer[i] = ith_element;
            }

            b.done_waiting();
        }
    };

    std::size_t length = std::distance(__first, __last);
    if (!length) { return; }

    std::vector<T> buffer(length);
    barrier b(length);

    std::vector<std::thread> threads(length - 1);
    join_threads joiner(threads);
    
    _Iterator block_start = __first; // unused?

    for (std::size_t i = 0; i != length - 1; ++i) {
        threads[i] = std::thread(process_element(), __first, __last, std::ref(buffer), i, std::ref(b));
    }

    process_element()(__first, __last, buffer, length - 1, b);
}
} // namespace par (parallel)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void printVec(const std::vector<int> &ivec) {
    for (int i : ivec) {
        std::cout << i << ' ';
    } std::cout << '\n';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::vector<int> ivec  = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    // std::vector<int> ivec(100);
    // std::iota(ivec.begin(), ivec.end(), 9);
    std::vector<int> ivec2(ivec.begin(), ivec.end());
    
    std::cout << "ivec: "; printVec(ivec);
    
    par::partial_sum(ivec.begin(), ivec.end());
    std::cout << "par:  "; printVec(ivec);
    
    std::partial_sum(ivec2.begin(), ivec2.end(), ivec2.begin());
    std::cout << "std:  "; printVec(ivec2);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// ivec: 1 2 3 4 5 6 7 8 9
// par:  1 3 6 10 15 21 28 36 45
// std:  1 3 6 10 15 21 28 36 45
// Program ended with exit code: 0
