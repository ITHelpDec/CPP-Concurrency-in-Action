#include <string>
#include <iostream>
#include <thread>

void f(int, const std::string&);

void oops(int some_param) {
    char buffer[1024];
    // 'sprintf' is deprecated: This function is provided for compatibility reasons only.
    // Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead.
    
    // sprintf(buffer, "%i", some_param);
    snprintf(buffer, 1024, "%i", some_param);
    
    // sends pointer to the buffer (the "oops" moment)
    // strong chance the function will exit before has been converted
    std::thread t(f, 3, buffer);
    t.detach();
}

void not_oops(int some_param) {
    char buffer[1024];
    snprintf(buffer, 1024, "%i", some_param);
    
    // using std::string avoids dangling pointer
    std::thread t(f, 3, std::string(buffer));
    t.detach();
}
