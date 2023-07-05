# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 09 - "Advanced thread management"

### The dream!
> _"The ideal scenario would be that you could divide the code into the smallest pieces that could be executed concurrently, pass them over to the compiler and library, and say, 'Parallelize this for optimal performance.'"_ – pg. 300

### Thread pools
A nice analogy comparing car pools to thread pools - everyone has access to the fleet, but if all cars are in use then you must wait until one becomes available.

> _"At its simplest, a thread pool is a fixed number of worker threads (`std::thread::hardware_concurrency()`)...there’s no way to wait for the task to complete...you have to manage the synchronisation yourself."_ – pg. 301

[thread_pool.cpp](thread_pool.cpp)

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
