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
### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
