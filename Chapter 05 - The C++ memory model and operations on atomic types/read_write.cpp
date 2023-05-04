#include <vector>
#include <thread>
#include <iostream>

std::vector<int> data = { 1, 2, 3, 4, 5 };

std::atomic<bool> data_ready(false);

void reader_thread() {
    std::cout << "(reader): ";
    
    int i = 1;
    while (!data_ready.load()) {
        std::cout << i++ << "...";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } std::cout << "the answer is " << data[0] << '\n';
}

void writer_thread() {
    std::cout << "(writer): sleeping for 3 sceonds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    data.push_back(69);
    data_ready = true;
}

void printVec(const std::vector<int> &ivec) {
    for (const auto e : ivec) {
        std::cout << e << ' ';
    } std::cout << '\n';
}

int main()
{
    std::cout << "(main): ivec = ";
    printVec(data);
    
    std::cout << "(main): launching writer and reader threads...\n";
    
    std::thread t1(writer_thread);
    std::thread t2(reader_thread);
    
    t1.join();
    t2.join();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (main): ivec = 1 2 3 4 5
// (main): launching writer and reader threads...
// (writer): sleeping for 3 sceonds...
// (reader): 1...2...3...the answer is 1
// Program ended with exit code
