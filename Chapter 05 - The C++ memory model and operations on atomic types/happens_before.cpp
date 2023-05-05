#include <iostream>

void print(int a, int b) {
    std::cout << a << ' ' << b << std::endl;
}

int get_num() {
    static int i = 0;
    return ++i;
}

int main()
{
    print(get_num(), get_num());
    
    // "order of evaluation of arguments to a function call is unspecified"
    // might be '1 2'; might be '2 1' (although every time i've run it, it's been '1 2'
    
    return 0;
}
