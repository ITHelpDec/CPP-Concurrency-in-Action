# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 01 - "Hello, world of concurrency in C++"

### Context switches
Context-switching is expensive.
> _"In order to perform a context switch, the OS has to save the CPU state and instruction pointer for the currently running task, work out which task to switch to, and reload the CPU state for the task being switched to."_
> "_The CPU will then potentially have to load the memory for the instructions and data for the new task into the cache, which can prevent the CPU from executing any instructions, causing further delay."_ – pg. 3

#
### Distributed systems
> _"Using separate processes for concurrency also has an additional advantage – you can run the separate processes on distinct machines connected over a network"_ – pg. 5

#
### Concurrency vs Parallelism
A nice way to define the differences between concurrency and parallelism.
> _"People talk about parallelism when their primary concern is taking advantage of the available hardware to increase the performance of bulk data processing, whereas people talk about concurrency when their primary concern is separation of concerns, or responsiveness"_ – pg. 7

#
### Why use concurrency?
There are two main reasons to use concurrency in an application (besides it being _Multithreading Monday_ 😄).
* Separation of concerns, and
* Performance

> _"The free lunch is over!"_ – Herb Sutter (great quote regarding the move from single-threaded operations)

#
### Embarrassingly parallel
* Task parallelism: divide a single task into parts and run each in parallel
* Data parallelism: each thread performs the same operation on different parts of the data

Embarrassingly parallel programmes have good scalability properties (i.e. "many hands make light work").

#
### Why not use concurrency?
When the cost outweighs the benefit.

#
### What do they all have in common?
__RAII__ (Resource Acquisition is Initialisation).

Locks ensure mutexes are unlocked when the relevant scope is exited.

#
### C++11
In C++11 we finally received a thread-aware memory model, as well as a multitude of other things.

#
### Atomics
> _"The support for atomic operations directly in C++ enables programmers to write efficient code with defined semantics without the need for platform-specific assembly language."_ – pg. 12

#
### Efficiency of the C++ Thread Library
__*The abstration penalty:*__
> _", ...it’s important to understand the implementation costs associated with using any high-level facilities, compared to using the underlying low-level facilities directly"_ – pg. 12

The C++ principle of zero-cost asbtractions still applies - you don't pay for what you don't use.

#
### `std::thread::native_handle()`
`std::thread::native_handle()` is a member function that allows the underlying implementation to be directly manipulated using a platform-specific API.

#
### "Hello, concurrent world!"
A basic example of how to run "Hello, world!" concurrently.

[hello_world.cpp](hello_world.cpp)

#
### Summary
Nice intro to the history and tidbits of concurrency - just a few notes'n'quotes to things that seemed worthwhile jotting down.
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
