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
    
    std::barrier sync(num_threads);
    std::vector<std::thread> threads(num_threads);
    
    std::vector<data_chunk> chunks;
    result_block result;
    
    for (std::size_t i = 0; i != num_threads; ++i) {
        threads[i] = std::thread( [&, i] () {
            while (!source.done()) {
                
                std::cout << std::this_thread::get_id() << " in while loop...\n";
                
                if (!i) {
                    data_block current_block = source.get_next_data_block();
                    chunks = divide_into_chunks(current_block, num_threads);
                }
                
                std::cout << std::this_thread::get_id() << " hit sync point 1..\n";
                sync.arrive_and_wait();
                std::cout << std::this_thread::get_id() << " synced 1!\n";
                
                result.set_chunk(i, num_threads, process(chunks[i]));
                
                std::cout << std::this_thread::get_id() << " hit sync point 2..\n";
                sync.arrive_and_wait();
                std::cout << std::this_thread::get_id() << " synced 2!\n";
                
                if (!i) { sink.write_data(std::move(result)); }
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
// Joining threads...
// 0x16fe870000x1703730000x1706bb000 in while loop...
// 0x16ff9f0000x1706bb0000x17025b000 hit sync point 1..
//  in while loop...
//  in while loop...
//  in while loop...
// 0x1703730000x17048b000 hit sync point 1..
//  in while loop...
// 0x170a030000x17048b000 in while loop...
//  hit sync point 1..
// 0x170a03000 hit sync point 1..
// 0x1707470000x1705a3000 in while loop...
// 0x1705a3000 hit sync point 1..
// 0x17002b0000x16ff13000 in while loop...
//  in while loop...
// 0x17002b0000x16ff13000 hit sync point 1..
// 0x1708eb000 in while loop...
// 0x16fe870000x1708eb000: thread 0 inside get_next_data_block()...
//  hit sync point 1..
//  in while loop...
// 0x16ff9f000 hit sync point 1..
//  hit sync point 1..
//  in while loop...
// 0x170747000 hit sync point 1..
// 0x17025b000 hit sync point 1..
// 0x16fe87000: thread 0 inside divide_into_chunks()...
// 0x16fe87000 hit sync point 1..
// 0x16fe87000 synced 1!
// 0x16fe87000: inside process()...
// 0x1703730000x1705a3000 synced 1!
// 0x1705a3000: inside process()...
// 0x170a03000 synced 1!
// 0x170a03000: inside process()...
// 0x1706bb000 synced 1!
// 0x1706bb000: inside process()...
//  synced 1!
// 0x170373000: inside process()...
// 0x1707470000x17025b000 synced 1!
// 0x17025b000: inside process()...
//  synced 1!
// 0x170747000: inside process()...
// 0x16ff9f000 synced 1!
// 0x17048b0000x16ff9f000 synced 1!
// 0x16ff13000 synced 1!
// 0x16ff130000x17002b000: inside process()...
//  synced 1!
// 0x17002b000: inside process()...
// 0x1708eb000 synced 1!
// 0x1708eb000: inside process()...
// : inside process()...
// 0x17048b000: inside process()...
// 0x16fe87000: inside set_chunk()...
// 0x1705a3000: inside set_chunk()...
// 0x170a03000: inside set_chunk()...
// 0x1706bb000: inside set_chunk()...
// 0x170373000: inside set_chunk()...
// 0x170747000: inside set_chunk()...
// 0x16ff130000x1708eb000: inside set_chunk()...
// 0x17025b000: inside set_chunk()...
// : inside set_chunk()...
// 0x17002b000: inside set_chunk()...
// 0x16ff9f000: inside set_chunk()...
// 0x17048b000: inside set_chunk()...
// 0x170a03000 hit sync point 2..
// 0x170373000 hit sync point 2..
// 0x16fe87000 hit sync point 2..
// 0x1705a3000 hit sync point 2..
// 0x1706bb000 hit sync point 2..
// 0x17025b0000x16ff9f000 hit sync point 2..
//  hit sync point 2..
// 0x17048b000 hit sync point 2..
// 0x16ff130000x17002b000 hit sync point 2..
//  hit sync point 2..
// 0x170747000 hit sync point 2..
// 0x1708eb000 hit sync point 2..
// 0x1708eb000 synced 2!
// 0x170747000 synced 2!
// 0x17002b000 synced 2!
// 0x170a03000 synced 2!
// 0x16ff13000 synced 2!
// 0x16ff9f000 synced 2!
// 0x17025b000 synced 2!
// 0x16fe87000 synced 2!
// 0x16fe87000: thread 0 inside write_data()...
// 0x17048b000 synced 2!
// 0x1706bb000 synced 2!
// 0x1705a3000 synced 2!
// 0x170373000 synced 2!
// Processed!
// Program ended with exit code: 0
