# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 05 - "The C++ memory model and operations on atomic types"

### Objects and memory locations
> _"The C++ Standard defines an object as 'a region of storage'"_ – pg. 125

> _"...though adjacent bit fields are distinct objects, they’re still counted as the same memory location."_ – pg. 125

I feel like the author glosses over bitfields (like we all know what they are), so if you're as unfamiliar with them as I am, then [here's](https://en.cppreference.com/w/cpp/language/bit_field) a link - they're not quite the same as `std::bitset`.

> _"If more than two threads access the same memory location, each pair of accesses must have a defined ordering."_ – pg. 127

> _"If there’s no enforced ordering between two accesses to a single memory location from separate threads, one or both of those accesses is not atomic, and if one or both is a write, then this is a data race and causes undefined behavior."_ – pg. 127

#
### Atomic operations prevent undefined behaviour
...but don't expect them to fix a data race.

#
### What is an atomic operation?
> _"An atomic operation is an inidivisible operation...it's either done or not done."_ – pg. 128

...followed by a lot of waffle...

> _"unsynchronised accesses to non-atomic variables form a simple problematic race condition...and cause undefined behavior."_ – pg. 128

> _"if the atomic operations themselves use an internal mutex then the hoped-for performance gains will probably not materialize"_ – pg. 128

#
### `::is_always_lock_free()`
> _"Since C++17, all atomic types have a static constexpr member variable, X::is_ always_lock_free, which is true if and only if the atomic type X is lock-free for all supported hardware that the output of the current compilation might run on."_ – pg. 129
```cpp
#include <atomic>
#include <iostream>

int main()
{
    std::atomic<int> ai = 3;
    std::cout << "ai.is_always_lock_free(): "
              << (ai.is_always_lock_free ? "true" : "false") << '\n'; // true
    
    return 0;
}
```

#
### `std::atomic_flag`
Operations on this type are _required_ to be lock-free.

They also have two functions as of when the book was written ([although there appear to more](https://en.cppreference.com/w/cpp/atomic/atomic_flag)):
* `test_and_set()` for `true`
* `clear()` for `false`

#
### `std::memory_order`
An enum that can be passed as an optional argument to the atomic type operations.
* `std::memory_order_relaxed`
* `std::memory_order_acquire`
* `std::memory_order_consume`
* `std::memory_order_acq_rel`
* `std::memory_order_release`
* `std::memory_order_seq_cst`

If no ordering is specified, the default ordering is used (`std::memory_order_seq_cst`).

#
### Worth remembering
* Store operations - relaxed, release, seq_cst
* Load operations - relaxed, consume, acquire, seq_cst
* Read-modify-wrte - all of the above

#
### Back to `std::atomic_flag`
If you're coding in C++11-C++17, you must always explicit initialise an atomic_flag to `ATOMIC_FLAG_INIT`.
```cpp
std::atomic_flag fleg = ATOMIC_FLAG_INIT;
```
From C++20 onwards, however, this macro is no longer needed (see [here](https://en.cppreference.com/w/cpp/atomic/ATOMIC_FLAG_INIT)).

[spinlock_mutex.cpp](spinlock_mutex.cpp)

If we go back to the two member functions:
* `.clear()` is a store operation, so we know what memory ordering we can take advantage of
* `.test_and_set()` is read-modify-write, so we have more semantics choices

The author mentions not being able to copy-contruct / perform assignment on atomics about five times, so probably also worth remembering.

#
### `std::atomic<bool>`
A less crippled version of `std::atomic_flag`.

[atomic_bool.cpp](atomic_bool.cpp)

#
### The "compare-exchange" principle
The cornerstone of programming with atomic types.

A bit of waffle followed by the introduction of two member functions:
* `.compare_exchange_weak()`
* `.compare_exchange_strong()`

#
### Spurious failure
Spurious failure is when a function fails; not because of values of the variable, but because of timing.

> _"Because `.compare_exchange_weak()` can fail spuriously, it must typically be used in a loop:"_ - pg. 135

> _"A failed compare-exchange doesn’t do a store, so it can’t have `std::memory_order_release` or `std::memory_order_acq_rel` semantics"_ – pg. 136

> _"You also can’t supply stricter memory ordering for failure than for success;...if you don’t specify an ordering for failure, it’s assumed to be the same as that for success"_ – pg. 136

We're falling victim to "feature dumping" in these couple of pages, with a lot of waffle and very little substance - yet to see these functions applied to something useful / relevant.

I'll be honest - after the waffle, I was none the wiser about how and when to apply these compare exchanges, but according to [cppreference](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange):

> _"Compare-and-exchange operations are often used as basic building blocks of lockfree data structures
"_

#
### Locking
`std::atomic<bool>` may not be lock-free.

#
### Atomic pointers (`std::atomic<T*>`)
We get two more member functions, ...
* `.fetch_add()`
* `.fetch_sub()`

...as well as the typical arithmetic operators (`+=`, `-=` and pre- and post-increment).

The biggest takeaway here is the subtle difference between the member functions and compound-assignment operators when it comes to pointer arithmetic:
* member functions - assign THEN step (returns old value)
* compound assignment = step THEN assign (returns new value)

[compound_fetch.cpp](compound_fetch.cpp)

#
### The std::atomic<> primary class template
You must have a trivial copy-assignment operator i.e. no virtual functions / base classes, and use the compiler-synthesised copy-assignment operator.

Every other base class and non-static data member must also have a trivial copy-assignment operator (allows the compiler to run `memcpy()`.

> _"..., it is worth noting that the compare-exchange operations do bitwise comparison as if using `memcmp`. rather than using any comparison operator..."_ – pg. 139

> _"In general, the compiler isn’t going to be able to generate lock-free code for `std::atomic<UDT>`, so it will have to use an internal lock for all the operations."_

Side-note: wrap all markdown angular brackets in code-wrappers or GitHub will auto-indent everytime you do a new line (frustratingAF).

If we supplied a user-defined copy-assignment operator then we'd be breaking the "don't pass a reference / pointer outside the scope of the lock" rule.
```cpp
T& operator=(const T &t);
```
Be mindful of `std::atomic<float>` and `std::atomic<float>` - the behaviour might be surprising.

#
### Non-member functions
More waffle followed by:
```cpp
std::atomic_is_lock_free(&x) == x.is_lock_free();
std::atomic_load(&x) == x.load();
std::atomic_load_explicit(&x, std::memory_order_acquire) = x.load(std::memory_order_acquire);
```
#
### `std::shared_ptr<>`
Not sure if this is the right kind of implementation, but I suppose this is the thing with books providing code samples with bodyless functions.

[atomic_shared.cpp](atomic_shared.cpp)

#
### Reader / writer
Inefficient as anything, but an example similar to the one we did with the [shared mutexes](https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/1f96b1f65f78e859dc41771a9658151dcf6a7c20/Chapter%2003%20-%20Sharing%20data%20between%20threads/README.md#protecting-rarely-updated-data-structures) (and maybe condition variables).

[read_write.cpp](read_write.cpp)

#
### Happens-before
Just as the _compare-exchange_ principle was the cornerstone of programming with atomic types, _"the happens-before and strongly-happens-before relationships are the basic building blocks of operation ordering in a program"_.

> _"it specifies which operations see the effects of which other operations"_ – pg. 145

[happens_before.cpp](happens_before.cpp)

Yet more waffle, followed by "don't use `std::memory_order_consume`", especially in strong-happens-before operations, so now we have a different problem - the code exmaples of these niche instances or no longer rubbish; they're non-existent!

#
### Memory ordering for atomic operations
Already covered this earlier, but now we're splitting them into three categories of ordering:
* sequentially-consistent (`seq_cst`)
* acquire-release (`consume`, `acquire`, `release`, `acq_rel`)
* relaxed (`relaxed`)

My understanding of this section is that memory orderings allow us to only "pay for what we need" in true C++ fashion with atomic operations.

#
### Explanations of memory ordering
* `std::memory_order_seq_csst` - like squashing all the instructions to make them appear as if they were performed sequentially on a single thread

> _"On a weakly-ordered machine with many processors, it can impose a noticeable performance penalty"_ – pg. 147

[seq_cst_example.cpp](seq_cst_example.cpp)

> _"For there to be a single total order, if one thread sees x==true and then subse- quently sees y==false, this implies that the store to x occurs before the store to y in this total order...it could also happen the other way round...but under no circumstance can z \[our atomic int\] be 0"_ – pg. 149

#
### Non-sequentially consistent memory orderings
> _"threads don’t have to agree on the order of events"_ – pg. 150

> _"In the absence of other ordering constraints, the only requirement is that all threads agree on the modification order of each individual variable."_ – pg. 150

#
### `std::memory_order_relaxed`
> _"Operations on atomic types performed with relaxed ordering don’t participate in synchronizes-with relationships (\[although\] they still obey happens-before relationships \[within a single thread\])."_ – pg. 150

> _"The only requirement is that accesses to a single atomic variable from the same thread can’t be reordered; once a given thread has seen a particular value of an atomic variable, a subsequent read by that thread can’t retrieve an earlier value of the variable"_ – pg. 150

[relaxed.cpp](relaxed.cpp)

In this instance (using `std::memory_order_relaxed`), the assertion _can_ fire.

> _"Even though there’s a happens-before relationship between the stores and between the loads, there isn’t one between either store and either load, and so the loads can see the stores out of order."_ – pg. 150

[relaxed_on_roids.cpp](relaxed_on_roids.cpp)

This is slightly more involved example of `std::memory_order_relaxed` - the atomic `go`is used to sync all of the threads; reason being is that _"launching a thread is expensive, and without the explicit delay, the first thread may finish before the last one has even started"_.

#
### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
