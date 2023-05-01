#include <vector>
#include <thread>
#include <barrier>
#include <iostream>
#include <random>
#include <future>

typedef std::vector<std::vector<int>> result_chunk;
typedef std::vector<std::vector<int>> data_chunk;
typedef std::vector<int> data_block;

bool fleg = false;

struct data_source {
    bool done()
    {
        auto f = std::async(std::launch::async, [this] () {
            set_fleg(fleg);
        });
        
        return fleg;
    }
    
    void set_fleg(bool &fleg)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        fleg = true;
    }
    
    data_block get_next_data_block()
    {
        std::cout << std::this_thread::get_id() << ": thread 0 inside get_next_data_block()...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return data_block();
    }
};

result_chunk process(const data_chunk &ivec) {
    std::cout << std::this_thread::get_id() << ": inside process()...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return result_chunk();
}

struct result_block {
    void set_chunk(std::size_t i, std::size_t num_threads, result_chunk r)
    {
        std::cout << std::this_thread::get_id() << ": inside set_chunk()...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
};

struct data_sink {
    result_block write_data(result_block &&result)
    {
        std::cout << std::this_thread::get_id() << ": thread 0 inside write_data()...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return result_block();
    }
};

std::vector<data_chunk> divide_into_chunks(data_block data, std::size_t num_threads) {
    std::cout << std::this_thread::get_id() << ": thread 0 inside divide_into_chunks()...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return std::vector<data_chunk>();
}

void process_data(data_source &source, data_sink &sink) {
    std::cout << "inside process_data()...\n";
    
    const std::size_t concurrency = std::thread::hardware_concurrency();
    const std::size_t num_threads = concurrency ? concurrency : 2;
    
    std::vector<data_chunk> chunks;
    
    auto split_source = [&] () {
        if (!source.done()) {
            std::cout << std::this_thread::get_id() << " inside divide_into_chunks()...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            data_block current_block = source.get_next_data_block();
            chunks = divide_into_chunks(current_block, num_threads);
        }
        
        std::cout << "source.done() == true for " << std::this_thread::get_id() << '\n';
    };
    
    split_source();
    
    result_block result;
    
    // std::experimental::flex_barrier no longer needed as of C++20
    std::barrier sync(num_threads, [&] () {
        std::cout << std::this_thread::get_id() << " inside sync()...\n";
        sink.write_data(std::move(result));
        split_source();
        return -1;
    });
    
    std::vector<std::thread> threads(num_threads);
    
    for (std::size_t i = 0; i != num_threads; ++i) {
        threads[i] = std::thread( [&, i] () {
            while (!source.done()) {
                std::cout << std::this_thread::get_id() << " in while loop...\n";
                
                result.set_chunk(i, num_threads, process(chunks[i]));
                
                std::cout << std::this_thread::get_id() << " hit sync point..\n";
                sync.arrive_and_wait();
                std::cout << std::this_thread::get_id() << " synced!\n";
            }
        });
    }
    
    std::cout << "Joining threads...\n";
    for (auto &&t : threads) { t.join(); }
}

int main()
{
    data_source source;
    data_sink sink;
    
    process_data(source, sink);
    
    std::cout << "Processed!\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// inside process_data()...
// 0x203d85b40 inside divide_into_chunks()...
// 0x203d85b40: thread 0 inside get_next_data_block()...
// 0x203d85b40: thread 0 inside divide_into_chunks()...
// source.done() == true for 0x203d85b40
// Joining threads...
// Processed!
// Program ended with exit code: 0
