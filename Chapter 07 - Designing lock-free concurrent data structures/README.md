# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 07 - "Designing lock-free concurrent data structures"

### Mutexes
> _"Mutexes are powerful mechanisms for ensuring that multiple threads can safely access a data structure without encountering race conditions or broken invariants."_ – pg. 205

#
### Definitions and consequences
> _"Algorithms and data structures that use mutexes, condition variables, and futures to synchronise the data are called blocking data structures and algorithms."_ – pg. 206

> _"Data structures and algorithms that don’t use blocking library functions are said to be non-blocking. Not all these data structures are lock-free, though, ..."_ – pg. 206

[spinlock_mutex.cpp](../Chapter%2005%20-%20The%20C++%20memory%20model%20and%20operations%20on%20atomic%20types/spinlock_mutex.cpp)

#### Obstruction-Free
> _"If all other threads are paused, then any given thread will complete its operation in a bounded number of steps."_ – pg. 207

#### Lock-Free
> _"If multiple threads are operating on a data structure, then after a bounded number of steps one of them will complete its operation."_ – pg. 207

#### Wait-Free
> _"Every thread operating on a data structure will complete its opera- tion in a bounded number of steps, even if other threads are also operating on the data structure."_ – pg. 207

#
### Lock-free data structures
> _"For a data structure to qualify as lock-free, more than one thread must be able to access the data structure concurrently."_ – pg. 207

> _"...if one of the threads accessing the data structure is suspended by the scheduler midway through its operation, the other threads must still be able to complete their operations without waiting for the suspended thread."_ – pg. 207

> _"If another thread performs operations with the “wrong” timing, the other thread might make progress but the first thread continually has to retry its operation. Data structures that avoid this problem are wait-free as well as lock-free."_ – pg. 207

#
### Wait-free data structures
> _"A wait-free data structure is a lock-free data structure with the additional property that every thread accessing the data structure can complete its operation within a bounded number of steps, regardless of the behavior of other threads."_ – pg. 208

> _"The scheduling of threads by the OS may mean that a given thread can loop an exceedingly large number of times, but other threads loop very few times. These operations are thus not wait-free."_ – pg. 208

#
### Does the benefit outweigh the cost?
> _"With a lock-free data structure, some thread makes progress with every step."_ – pg. 208

>  _"With a wait-free data structure, every thread can make forward progress, regardless of what the other threads are doing; there's no need for waiting."_ – pg. 208

This is very important!

> _"If a thread dies while holding a lock, that data structure is broken **forever**."_ – pg. 208

> _"To avoid the undefined behavior associated with a data race, you must use atomic operations for the modifications...\[and\]...you must ensure that changes become visible to other threads in the cor- rect order."_ – pg. 208

#
### Live locks (as opposed to deadlocks)
A live lock is like [the chopstick scene from Kung Fu Panda](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwip_oH4xpj_AhUMB8AKHYFpBQ8QtwJ6BAgHEAI&url=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3DBE1zdZYqovQ&usg=AOvVaw09RiB4FOyjzsPEDh0njpJq) - two threads or more are trying to act on the same object at once, but unless one concedes to the other, they will continue to clash.

> _"By definition, wait-free code can’t suffer from live lock because there’s always an upper limit on the number of steps needed to perform an operation."_ – pg. 209

> _", ...the cache ping-pong associated with multiple threads accessing the same atomic variables can be a significant performance drain."_ – pg. 209

> _"On some platforms, what appears to be lock-free code might be using locks internal to the C++ Standard Library implementation"_ – pg. 210

#
### A lock-free stack
Nodes are retrieved in the reverse order they were added (LIFO - Last In, First Out).

The simplest stack is a linked list - adding a node is as follows:
1) Create a new node
2) Set its next pointer to the current head node
3) Set the head node to point to it

...although the potential for a race condition lies between steps 2 and 3.

> _"It’s therefore vital that your new node is thoroughly prepared before head is set to point to it; you can’t modify the node afterward."_ – pg. 210

[lock_free_stack.cpp](lock_free_stack.cpp)

A trickier example to play around with, this one - I found it odd that this example strayed away from the heavy use of smart pointers, but reading further on into the book we see talk of memory leaks and reclamation.

I made a few tweaks - perfect-forwarding for `.push(D &&data)`, a barrier to synchronise the threads before pushing both lvalues and rvalues to see if it would produce any synchronisation errors, and then a simple traverse-a-linked-list `.print()` function, although I wonder if this needs to be handled atomically like `.push()` in case the data structure is being modified while it's being read.

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
