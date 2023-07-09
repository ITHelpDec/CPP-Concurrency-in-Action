# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 09 - "Advanced thread management"

### The dream!
> _"The ideal scenario would be that you could divide the code into the smallest pieces that could be executed concurrently, pass them over to the compiler and library, and say, 'Parallelize this for optimal performance.'"_ – pg. 300

### Thread pools
A nice analogy comparing car pools to thread pools - everyone has access to the fleet, but if all cars are in use then you must wait until one becomes available.

> _"At its simplest, a thread pool is a fixed number of worker threads (`std::thread::hardware_concurrency()`)...there’s no way to wait for the task to complete...you have to manage the synchronisation yourself."_ – pg. 301

[thread_pool.cpp](thread_pool.cpp)

We need to use the lock-based thread_safe queue from Chapter 6 and before for this to work (unfortunately, the lock-free queue will not work, as we have not created a `.try_pop()` member function, but we could create one if we wanted).

The `.submit()` function passes a function onto our multithreaded worker queue for processing.

We must also pay attention to the order of our member declarations to ensure destruction happens in the right order.

> _"...both the `done_` flag and the `workq_` must be declared before the `threads_` vector, which must in turn be declared before the `joiner_`."</br>"This ensures that the members are destroyed in the right order; you can’t destroy the queue safely until all the threads have stopped, for example."_ – pg. 303

> _"For many purposes this simple thread pool will suffice, especially if the tasks are entirely independent and don’t return any values or perform any blocking operations."_ – pg. 303

### Waiting for tasks to finish
> _"Because std::packaged_task<> instances are not copyable, just movable, you can no longer use std::function<> for the queue entries...instead, you must use a custom function wrapper that can handle move-only types"_ – pg. 304

[waitable_thread_pool.cpp](waitable_thread_pool.cpp)

I'll take the author's word that this is the easiest way to handle passing a a move-only `std::packaged_task<>` - a few similar throwbacks to previous exampmles with opting for `std::invoke_result_t` over the deprecated `std::result_of`, and choosing `std::make_unique` over raw calls to `operator new` for better exception safety ([PR](https://github.com/anthonywilliams/ccia_code_samples/pull/46) here).

One thing that stood out, though, is the data race when adding tasks to our multithreaded queue - on quite a few instances there was undefined behaviour when trying to square the contents of a 10-element vector. It could be the implementation I'm using, or even my function, but regardless, it's not working as intended.

The key takeaway is that the use of `std::future<T>` allows us to wait for our task to complete.

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
