#include <condition_variable>
#include <mutex>
#include <chrono>
#include <iostream>

std::condition_variable cv;
bool done;
std::mutex m;

bool wait_loop() {
    std::cout << "Creating timeout period 5000ms from now...\n";
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(5000);
    
    {   // first time author hasn't included a std::unique_lock in its own scope?
        std::cout << "Locking mutex...\n";
        std::unique_lock lock(m);
        
        std::cout << "Waiting until 5000ms has passed, then break while loop...\n";
        while (!done) {
            if (cv.wait_until(lock, timeout) == std::cv_status::timeout)
                break;
        }
    }
    
    return done;
}

int main()
{
    wait_loop();
    
    return 0;
}
