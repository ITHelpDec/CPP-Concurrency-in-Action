#include <vector>
#include <iostream>
#include <numeric>

void printVec(const std::vector<int> &ivec) {
    for (int i : ivec) {
        std::cout << i << ' ';
    } std::cout << '\n';
}

int main()
{
    std::vector<int> invec = { 1, 2, 3 };
    std::vector<int> outvec(invec.size());
    
    std::cout << "invec:  "; printVec(invec);
    std::cout << "outvec: "; printVec(outvec);
    
    std::partial_sum(invec.begin(), invec.end(), outvec.begin());
    
    std::cout << "invec:  "; printVec(invec);
    std::cout << "outvec: "; printVec(outvec);
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// invec:  1 2 3
// outvec: 0 0 0
// invec:  1 2 3
// outvec: 1 3 6
// Program ended with exit code: 0
