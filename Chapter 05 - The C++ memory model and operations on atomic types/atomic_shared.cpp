#include <memory>
#include <iostream>

std::shared_ptr<int> sp = std::make_shared<int>(5);

void process_data(std::shared_ptr<int> sp) {
    *sp = 42;
}

void process_global_data() {
    std::cout << "Inside process_global_data()";
    
    std::cout << "...assigning with std::atomic_load()...\n";
    std::shared_ptr<int> local = std::atomic_load(&sp);
    
    std::cout << "local: " << *local.get() << '\n';
    std::cout << "sp.use_count(): " << sp.use_count() << '\n';
    std::cout << "local.use_count(): " << local.use_count() << "\n\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "Modifying contents...\n";
    process_data(local);
    
    std::cout << "sp:    " << *sp.get()    << '\n';
    std::cout << "local: " << *local.get() << '\n';
    
    std::cout << "sp.use_count(): " << sp.use_count() << '\n';
    std::cout << "local.use_count(): " << local.use_count() << "\n\n";
}

void update_global_data() {
    std::cout << "Inside update_global_data()";
    
    std::cout << "...creating new shared_ptr...\n";
    
    // opt for std::make_shared
    // std::shared_ptr<int> local(new my_data);
    auto local = std::make_shared<int>(69);
    
    std::cout << "local: " << *local.get() << '\n';
    
    std::cout << "sp.use_count(): " << sp.use_count() << '\n';
    std::cout << "local.use_count(): " << local.use_count() << "\n\n";
    
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    std::cout << "Storing local to sp with atomic_store...\n";
    std::atomic_store(&sp, local);
    
    std::cout << "sp:    " << *sp.get()    << '\n';
    std::cout << "local: " << *local.get() << '\n';
    
    std::cout << "sp.use_count(): " << sp.use_count() << '\n';
    std::cout << "local.use_count(): " << local.use_count() << "\n\n";
}

int main()
{
    std::cout << "Our global shared_ptr, sp: " << *sp.get() << "\n\n";
    
    process_global_data();
    
    update_global_data();
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Our global shared_ptr, sp: 5

// Inside process_global_data()...assigning with std::atomic_load()...
// local: 5
// sp.use_count(): 2
// local.use_count(): 2

// Modifying contents...
// sp:    42
// local: 42
// sp.use_count(): 2
// local.use_count(): 2

// Inside update_global_data()...creating new shared_ptr...
// local: 69
// sp.use_count(): 1
// local.use_count(): 1

// Storing local to sp with atomic_store...
// sp:    69
// local: 69
// sp.use_count(): 2
// local.use_count(): 2

// Program ended with exit code: 0
