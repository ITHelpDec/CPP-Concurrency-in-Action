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

### Waitable parallel accumulate
Another instance of poorly-tested code from listing 9.3 - PR is [here](https://github.com/anthonywilliams/ccia_code_samples/pull/48).

[waitable_accumulate.cpp](waitable_accumulate.cpp)

### Pending tasks
Similar to the loop of the worker thread, this `run_pending_task` function tries to take a task off the queue and run it if there is one - otherwise, it yields to the OS to reschedule the thread.

We have two more examples of code that is a bit all over the show from the author, with non-existent discrepancies [here]([url](https://github.com/anthonywilliams/ccia_code_samples/issues/49)) and overall non-functioning code [here]([url](https://github.com/anthonywilliams/ccia_code_samples/issues/50)) for listing 9.5.

### Avoiding contention in the work queue
Because there's only one data source...
> _", ...as the number of processors increases, there’s increasing contention on the queue..."_ – pg 310

This can be a huge performance drain due to cache ping-pong (even if you've opted for a lock-free queue to avoid explicit waiting).

> _"One way to avoid cache ping-ponging is to use a separate work queue per thread"_ – pg. 310

(I'll maybe look into implementing this at a later date).

> _"This works fine for reducing contention, but when the distribution of work is uneven, it can easily result in one thread having a lot of work in its queue while the others have no work to do"_ – pg. 311

An interesting example of this uneven distribution would be with "quick sort", but, in the next section, we will learn to "steal" work from other threads to ensure appropriate load balancing.

### Work stealing

In order for this to work, the saturated queue must be accessible to the thread doing the stealing - the way to do this is by having each thread register its queue with the thread pool (or be given one by the thread pool).

We must also "ensure that the data in the work queue is suitably synchronised and protected so that (y)our invariants are protected" (pg. 311).

One suggestion from the other is to have a lock-free double-ended queue that allows the owner thread to push and pop from one end of the queue, while the other threads can steal from the other end.

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
