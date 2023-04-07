#include <mutex>
#include <queue>
#include <condition_variable>

std::mutex m;

// wth is a data chunk??
// std::queue<data_chunk>

std::queue<int> iq;
std::condition_variable cv;

void data_prep_thread() {
    while (more_data_to_process()) {
        const int data = prepare_data();
        
        {
            std::lock_guard<std::mutex> lock(m);
            iq.push(data);
        } // lock(m)
        
        cv.notify_one();
    }
}

void data_proc_thread() {
    while (true) {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [] () { return !iq.empty(); } );
        
        int data = iq.front();
        iq.pop();
        
        lock.unlock();
        
        process(data);
        
        if (is_last_chunk) { break; }
    }
}
