#include <atomic>
#include <vector>
#include <thread>
#include <iostream>

std::vector<int> queue_data;
std::atomic<int> count;

void wait_for_more_items() {
    std::cout << "(waiting) " << std::this_thread::get_id() << '\n';
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void populate_queue() {
    constexpr std::size_t num_of_items = 20;
    queue_data.clear();
    
    for (int i = 0; i != num_of_items; ++i) { queue_data.push_back(i); }
    
    count.store(num_of_items, std::memory_order_release);
}

void process(int i) {
    std::cout << "(process) " << std::this_thread::get_id() << '\n';
}

void consume_items() {
    while (true) {
        int item_index;
        if ((item_index = count.fetch_sub(1, std::memory_order_acquire)) <= 0) {
            wait_for_more_items();
            continue;
        }
        process(queue_data[item_index - 1]);
    }
}

int main()
{
    std::thread t1(populate_queue);
    std::thread t2(consume_items);
    std::thread t3(consume_items);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (waiting) 0x16ff13000
// (waiting) 0x16ff9f000
// (process) 0x16ff13000
// (process) 0x16ff13000
// (process) 0x16ff13000
// ...
// (waiting) 0x16ff13000
// (waiting) 0x16ff9f000
// (waiting) 0x16ff13000
// ...
