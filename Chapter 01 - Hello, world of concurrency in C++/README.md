# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 01 - "Hello, world of concurrency in C++"

### Context switches
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
### ...work in progress
