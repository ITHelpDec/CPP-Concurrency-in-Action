#include <latch>
#include <future>
#include <iostream>

int make_data(int i) { return i * 2; }

void do_more_stuff() {
    std::cout << "lazy coders don't complete their functions...\n";
}

void process_data(const std::vector<int> &ivec, std::size_t thread_count) {
    std::cout << "inside process_data()\n";
}

void foo() {
    const std::size_t thread_count = std::thread::hardware_concurrency();
    std::cout << "thread_count = " << thread_count << '\n';
    
    std::latch done(thread_count);
    
    std::vector<int> data(thread_count); // opted for std::vector<T> over vague array
    std::vector<std::future<void>> threads;
    
    for (int i = 0; i != thread_count; ++i) {
        threads.emplace_back(std::async(std::launch::async, [&, i] () {
            data[i] = make_data(i);
            done.count_down(); // introduced in macOS 11.0
            do_more_stuff();
        }));
    }
    
    done.wait();
    process_data(data, thread_count);
}

int main()
{
    foo();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// thread_count = 12                            (done.count() = 12)
// lazy coders don't complete their functions...(done.count() = 11)
// lazy coders don't complete their functions...(done.count() = 10)
// lazy coders don't complete their functions...(done.count() = 9)
// lazy coders don't complete their functions...(done.count() = 8)
// lazy coders don't complete their functions...(done.count() = 7)
// lazy coders don't complete their functions...(done.count() = 6)
// lazy coders don't complete their functions...(done.count() = 5)
// lazy coders don't complete their functions...(done.count() = 4)
// lazy coders don't complete their functions...(done.count() = 3)
// lazy coders don't complete their functions...(done.count() = 2)
// lazy coders don't complete their functions...(done.count() = 1)
// inside process_data()                        (done.count() = 0)
// lazy coders don't complete their functions...
// Program ended with exit code: 0
