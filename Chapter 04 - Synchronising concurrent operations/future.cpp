#include <future>
#include <iostream>

int giggedy() {
    std::cout << "(giggedy): sleeping asynchronously for 15 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(15));
    return 69;
}

void woof_ten_times() {
    std::cout << "(woof): woofing while we wait for the meaning of life...\n";
    for (auto i = 0; i != 5; ++i) {
        std::cout << "woof..." << i+1 << '\n';
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main()
{
    std::future<int> meaning_of_life = std::async(giggedy);
    
    woof_ten_times();
    
    std::cout << "(main): The meaning of life is..." << meaning_of_life.get() << std::endl;
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// (woof): woofing while we wait for the meaning of life...
// woof...1
// (giggedy): sleeping asynchronously for 15 seconds...
// woof...2
// woof...3
// woof...4
// woof...5
// (main): The meaning of life is...69
// Program ended with exit code: 0
