#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>
#include <thread>

std::mutex m;
std::condition_variable cv;
std::string data;

bool ready, processed;

void worker_thread() {
    std::unique_lock lock(m);
    cv.wait(lock, [] () { return ready; } );
    
    std::cout << "Worker thread will sleep for a few seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    std::cout << "Worker thread is now processing data...\n";
    data += " after processing.\n";
    
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
    
    // manual unlock required before notification
    lock.unlock();
    cv.notify_one();
}

int main()
{
    std::thread worker(worker_thread);
    
    data = "Example data";
    
    {
        // send data to the worker thread (setting `ready` equal to `true`)
        std::lock_guard lock(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    
    cv.notify_one();
    
    {
        // wait for the worker
        std::unique_lock lock(m);
        cv.wait(lock, [] () { return processed; } );
    }
    
    std::cout << "Back in main(), data = " << data << '\n';
    
    worker.join();
    
    return 0;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// main() signals data ready for processing
// Worker thread will sleep for a few seconds...
// Worker thread is now processing data...
// Worker thread signals data processing completed
// Back in main(), data = Example data after processing.
//
// Program ended with exit code: 0
