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

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
