# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 08 - "Designing concurrent code"

### To generalise or to specialise
> _"You need to decide whether to have “generalist” threads that do whatever work is necessary at any point in time or “specialist” threads that do one thing well, or some combination"_ – pg. 253

#
### Divide and conquer
> _"...the worker threads run these tasks independently, and the results are combined in a final reduction step"_ – pg. 253

> _"If you’re going to parallelise this algorithm, you need to make use of the recursive nature."_ – pg. 254

> _"By using `std::async()`, you ask the C++ Thread Library to decide when to run the task on a new thread and when to run it synchronously._ – pg. 245

#
### Start as we mean to go on...
I have spent far too much time debugging Listing 8.1 (issue and PR are [here](https://github.com/anthonywilliams/ccia_code_samples/issues/39) and [here](https://github.com/anthonywilliams/ccia_code_samples/pull/40)).

Combining Listing 6.1 and Listing 8.1 with the changes suggested above, we have a functioning piece of code that sorts a simple list, but it still seems to have issues every so often, especially with `-fsanitize=thread` enabled in Xcode.

[six_and_eight_sorter.cpp](six_and_eight_sorter.cpp) | [ts_stack_sorter.cpp](ts_stack_sorter.cpp)

A few things to bear in mind with these implementations is the importance of `std::move` when reading from teh top of our stack - without it, we will presented with an obscure compiler error that took far too long to debug.
```
No matching function for call to 'construct_at`
```
I've also rejigged the implementation from a `struct` to a `class`, keeping only `do_sort()` as a public member function; nothing else is called publicly - beyond the constructor and destructor, they're all just helper functions / private member variables).

We have similar issues using our lock-free "ref-clamation" stack from the previous chapter, but – again – this only works 8-9 times out of 10.

[lf_stack_sorter.cpp](lf_stack_sorter.cpp)

I spent far too long putting these examples together after the PR, but I wanted to make sure I was able to move on with functioning code samples.

---
On a personal note...

> _"As with most of the examples, this is intended to demonstrate an idea rather than being production-ready code.""

...from pg. 255 really isn't good enough - no book should be published with this many mistakes and bad practices under the guise of it not being "production-ready".

It's instructional, and as such should not only compile and run, but run as intended.

#
### Time to specialise
> _"An alternative to dividing the work is to make the threads specialists, where each per- forms a distinct task, just as plumbers and electricians perform distinct tasks when building a house."_ – pg. 258

> _"There are two big dangers with separating concerns with multiple threads. The first is that you’ll end up separating the wrong concerns."_ – pg. 259

> _"if two threads are communicating a lot with each other but much less with other threads, maybe they should be combined into a single thread."_ – pg. 259

#
### Pipelines

> _"If your task consists of applying the same sequence of operations to many independent data items, you can use a pipeline..."_ – pg. 259

There's a lot of waffle in these pages...

#
### `std::thread::hardware_concurrency()`
One reason to opt for `std::async` (or careful use of thread pools):
> _"Using `std::thread::hardware_concurrency()`...your code doesn’t take into account any of the other threads that are running on the system unless you explicitly share that information."_ – pg. 261

#
### Contention
> _"If two threads are executing concurrently on different processors and they’re both reading the same data, this usually won’t cause a problem; the data will be copied into their respective caches, and both processors can proceed."</br>"But if one of the threads modifies the data, this change then has to propagate to the cache on the other core, which takes time."_ – pg. 262

Blah, blah, blah, cores waiting on each other = contention, happens with mutexes, etc., etc., etc., ...

The author is suggesting a lot of "here's a thing, but here's a problem, and you probably won't resolve this problem, so you're stuck with it" - thing is, though, I don't want just problems; I want best practices.

#
### False sharing
Cache lines are (currently) typically 32 or 64 bytes wide.

Set of data access by thread on same cache line? Está bien.

Unrelated data items on cache lines to be accessed by different threads? No bueno.

Thread wants to changes value on cache line? Takes onwership, then transfers ownership to next thread.

> _"The cache line is shared, even though none of the data is, hence the term false sharing"_ – pg. 264

Solution? Put same thread items on same cache line.

We see reference to `std::hardware_destructive_interference_size` again (like from C++ High Performance [here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2011%20-%20Concurrency/README.md) and [here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2011%20-%20Concurrency/false_sharing.cpp)).

Again, more waffle...more threads than cores = _task switching_...increased pressure on cache...reference to the aforementioned's counterpart (`std::hardware_constructive_interference_size` - max number of consecutive bytes guaranteed to be on the same cache line).

We're like 10 pages in now - less talky, more code-y...

#
### Oversubscription
> _"Oversubscription can arise when you have a task that repeatedly spawns new threads without limits, .."_ – pg. 266

#
### "Designing data structures for multi-threaded performance"
This is addressed, but I'm  still not sure why this is appearing again, considering we've spent the last two chapter doing this with lock-based and lock-free approach.

Regardless, three things to bear in mind when designing our data structures are:
* contention
* false sharing
* data proximity

#
### Matrices
We have the classic matrix multiplication crux.

> _"...it’s clear that you ought to be accessing adjacent columns, so the N elements from each row are adjacent, and you minimize false sharing."_

We can invert the x by y approach to avoid cache-thrashing like we've done before ([here](https://github.com/ITHelpDec/CPP-Primer) and [here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2004%20-%20Data%20Structures/loop_interchange.cpp#L19)), but I'm hoping we'll take a multi-threaded approach to this.

> _"If the space occupied by your N elements is an exact number of cache lines, there’ll be no false sharing because threads will be working on separate cache lines."_ – pg. 268

> _"On the other hand, if you have each thread compute a set of rows, ...it has a contiguous block of memory that’s not touched by any other thread."_ – pg. 268

> _"Dividing into rectangular blocks...has the same false-sharing potential as division by columns."_ – pg. 268

The example used is a 1,000 x 1,000 matrix (1,000,000 elements) across 100 processors:

10 rows each get us to 10,000, but to calculate those 10,000 results we need access to the entirety of the second matriax (another 1,000,000 elements) - total of 1,010,000 elements.

However...if we do 100 x 100 blocks (still 10,000 elements), the accesses are 100 from one matrix (100 x 1,000) and 100 from another.

This is 200,000 in total, and 200,000 vs 1,010,000 is a 5x reduction in reads.

Fewer reads means less likelihood of cache misses.

Great theory, but it would have nice to see actual code - again, "less talky, more codey".

> _"...look at all the aspects of the data access patterns carefully, and identify the potential causes of performance hits."_ – pg. 269
 
#
### Data access pattern considerations
* Ensure contiguous data is worked on by the same thread
* Minimise data required by any given thread
* Ensure data accessed by separate threads is sufficiently far apart to avoid false sharing (`std::hardware_destructive_interference_size` will help with this)

> _"...having data end up in different places on the heap isn’t a particular problem in itself, but it does mean that the processor has to keep more things in cache - this can be beneficial"_ – pg. 269

There's a good piece of info on using pointers in a tree with threads.

> _"If multiple threads need to traverse the tree, then they all need to access the tree nodes, but if the tree nodes only contain pointers to the real data held at the node, then the processor only has to load the data from memory if it’s needed."</br>"If the data is being modified by the threads that need it, this can avoid the performance hit of false sharing between the node data itself and the data that provides the tree structure."_ – pg. 269

To reiterate what the author mentioned, if we're using mutexes, then it's a good idea if both the mutexes and data are in close proximity to each other, although there can be a performance hit if other threads try to lock the mutex while it's held by the first thread.

#
### "Cushion for the pushin" (of improved mulithreading performance)
We can add large blocks of padding between mutexes and data members to test if either mutex contention of false sharing is affecting performance.
```cpp
template <typename T>
struct mutex_contention {
    std::mutex m_;
    char padding[std::hardware_destructive_interference_size];
    std::vector<T> data_;
};

template <typename T>
struct false_sharing {
    std::vector<T> v1;
    std::vector<T> v2;
    char padding[std::hardware_destructive_interference_size];
};
```
I found out the hard way back in C++ High Performance that we can also use `_X86_INSTRUCTION_STATE_CACHELINE_SIZE` on Intel Mac's where `std::hardware_destructive_interference_size` commnand isn't available.
```cpp
char padding[_X86_INSTRUCTION_STATE_CACHELINE_SIZE * 1024];
```

#
### Scaling
> _"Code is said to be scalable if the performance increases as more processing cores are added to the system"_ – pg. 270-271

#
### Exceptions
We can allow exceptions to propogate to the caller in sequential algorithms - we can't in parallel algorithms because it would be on the wrong call stack.

> _"If a function spawned on a new thread exits with an exception, the application is terminated."_ – pg. 271

We can see this with the `std::accumulate` example supplied in the book (although, yet again, [another PR](https://github.com/anthonywilliams/ccia_code_samples/pull/41) was needed for code that doesn't look like it was tested):

[par_accumulate.cpp](par_accumulate.cpp)

The first exception to watch out for is in the construction of our threads (same goes for `accumulate_block`:

> _"...the destructors of your new `std::thread` objects will call `std::terminate` and abort your programme."_ – pg. 272

We can use `std::packaged_task` to help handle our exception dilemma.

[packaged_accumulate.cpp](packaged_accumulate.cpp)

After rewriting and correcting this many examples, I don't know if can take this book seriously any more - the amount of mistakes is just mental...I've uploaded another PR [here](https://github.com/anthonywilliams/ccia_code_samples/pull/42) and decided to introduce some range-based `for` loops to sync our threads and futures (not sure why index-based for loops were still considered a good idea in 2017).

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
