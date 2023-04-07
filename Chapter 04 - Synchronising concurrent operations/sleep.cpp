#include <mutex>
#include <thread>
#include <iostream>

bool flag;
std::mutex m;

void wait_for_flag() {
    std::unique_lock<std::mutex> lock(m);
    while (!flag) {
        lock.unlock();
        std::cout << '.';
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        lock.lock();
    } std::cout << '\n';
}

int sleep_time = 5;

void set_flag_after(int sleep_time) {
    std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
    flag = true;
}

int main()
{
    std::cout << "flag: " << std::boolalpha << "\"" << flag << "\"\n";
    
    std::cout << "Setting timer of " << sleep_time << " seconds to set flag to true...\n";
    
    std::thread t1(set_flag_after, sleep_time);
    std::thread t2(wait_for_flag);
    
    t1.join();
    t2.join();
    
    std::cout << sleep_time << " seconds have now passed - our flag is now \"" << flag << "\"\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// flag: "false"
// Setting timer of 5 seconds to set flag to true...
// ................................................
// 5 seconds have now passed - our flag is now "true"
// Program ended with exit code: 0
