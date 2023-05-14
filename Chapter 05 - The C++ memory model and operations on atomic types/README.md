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
* `std::memory_order_seq_cst` - like squashing all the instructions to make them appear as if they were performed sequentially on a single thread

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

The author's description of people on the other end of the phone writing to and reading numbers off of a notepad in succession, but never going back on themselves as they progress through the list, is quite a clever analogy - have to give credit where credit is due.

> _"Write down this number, and tell me what was at the bottom of the list = `.exchange()`"_ – pg. 155

> _"Write down this number if the number on the bottom of the list is *that*; otherwise tell me what I should have guessed = `.compare_exchange_strong()`"_ – pg. 155

It would be nice, however, to know under what circumstances this memory ordering is useful.

I think it's great demonstrating what things do, but without context and relevance it's just yet another feature - a feature that the author then recommends we don't use after 7-odd pages of describing all of its intricacies.

Why do we ignore the why??

Given the unpredictable nature of the ordering, you'd almost wonder if it would be a good way of generating non-psuedo random numbers, even though there are plenty of methods in the `<random>` library?

#
### Acquire-release ordering
> _"One way to achieve additional synchronisation without the overhead of full-blown sequential consistency is to use acquire-release ordering"_ – pg. 155

* `.load()` calls are _acquire_ operations (`std::memory_order_acquire`)
* `.store()` calls are _release_ operations (`std::memory_order_release`)
* _read-modify-write_ operations like `.fetch_add()` and `.exchange()` can be either _acquire_, _release_, or both (`std::memory_order_acq_rel`)

> _"A release operation synchronises with an acquire operation that reads the value written"_ – pg. 156

[acq_rel.cpp](acq_rel.cpp)

The author also introduces another example, this time where two `.store()` operations are called on the same thread (one with relaxed ordering, the other with release).

> _"In order to provide any synchronization, acquire and release operations must be paired up."_ – pg. 158

> _"The value stored by a release operation must be seen by an acquire operation for either to have any effect"_ – pg. 158

[acq_rel_same_thread.cpp](acq_rel_same_thread.cpp)

> _"Now, here’s where the acquire-release semantics kick in: if you tell the man all the batches you know about when you ask for a value, he’ll look down his list for the last value from any of the batches you know about and either give you that number or one further down the list."_ – pg. 158

This is quite clever, and what feels like another good analogy from the author - with this description, it almost sounds like an associative container, with the value being a stack / vector of potential values (in this case).

The acquire-release model appears to write to and read from the **_last_** value of the "batch" - this helps to syncrhonise data across multiple threads.

Clever!

#
### Transitive ordering & synchronisation
Below we have an example of the "store-release, load-acquire" semantics in action across threads.

[three_blind_threads.cpp](three_blind_threads.cpp)

`sync1` "passes the baton" from thread 1 to thread 2, then `sync2` passes the baton from thread 2 to thread 3.

This can be simplified into a single variable by taking advantage of "read-modify-write" i.e. `std::memory_order_acq_rel` in the intermediatary steps (thread 2) - my one question, though, is - why use `.compare_exchange_strong()` in a loop if `compare_exchange_weak()` was recommended in looping instances earlier in the book at pg. 135?

> _"`.compare_exchange_strong()`...can eliminate the need for loops...where you want to know whether you successfully changed a variable or whether another thread got there first."_

[read_modify_write.cpp](read_modify_write.cpp)

One thing I may have missed about why to opt for weak or strong – if the calculation of the value to be stored is _cheap_, opt for _weak_; if it's **expensive**, opt for **strong** (pg. 136).

A main takeaway from this section is that `std::memory_order_acq_rel` can synchronise with a prior _store-release_ or with a subsequent _load_acquire_.

#
### Mixing it up
If you mix acquire-release operations with consistently-sequential operations, sequentially-consistent operations fall in with the acquire-release operations and mimic their semantics ("store-release, load-acquire").

#
### Mutexes
Like mutexes, your acquire and release operations have to be on the same variable to ensure an ordering (161).

#
### Acquire-release is cheaper than sequentially-consistent
> _"If you don’t need the stringency of sequentially consistent ordering for your atomic operations, the pair-wise synchronisation of acquire-release ordering has the potential for a much lower synchronisation cost than the global ordering required for sequentially consistent operations."_ – pg. 161

#
### `std::memory_order_consume`
The C++17 standard explicitly recommends that you don't use it, with its unique trait being that it's all about dependencies...but what is a data dependency?

> _"...there is a data dependency between two operations if the second one operates on the result of the first."_ – pg. 162

> _"One important use for this kind of memory ordering is where the atomic operation loads a pointer to some data."_ – pg. 162

> _"By using `std::memory_order_consume` on the load and `std::memory_order_release` on the prior store, you ensure that the pointed-to data is correctly synchronized, without imposing any synchronisation requirements on any other non-dependent data."_ – pg. 162

[consume.cpp](consume.cpp)

I attempted using smart pointers with this example, but didn't have much luck - it seems to be the norm to call `operator new` when using `std::memory_order_consume`. I get that you need to use variables on the heap to allow inter visiblity across threads, but this looks like a memory leak waiting to happen. `std::atomic<std::shared_ptr<T>>` exists as of C++20, but compatibility across compilers still hasn't improved since the last time I tried in C++ High Performance ([here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2011%20-%20Concurrency/atomic_shared_ptr.cpp#L7));

#
### `std::kill_dependency()`
> _"if you have a global read-only array, and you use `std::memory_order_consume` when retrieving an index into that array from another thread, you can use `std::kill_dependency()` to let the compiler know that it doesn’t need to reread the contents of the array entry."_ – pg. 163

[kill_dependency.cpp](kill_dependency.cpp)

I added an extra store-release function (`e()`) with some extra threads to make it more similar to the previous examples.

#
### Release sequences
> _"There can be any number of links in the chain, but provided they’re all read-modify-write operations such as `.fetch_sub()`, the `.store()` will still synchronize with each one that’s tagged `std::memory_order_acquire`."_ – pg. 165

[sequence.cpp](sequence.cpp)

#
### Fences (aka memory barriers)
> _"These are operations that enforce memory-ordering constraints without modifying any data and are typically combined with atomic operations that use the `std::memory_order_relaxed` ordering constraints."_ – pg. 166

> _"Fences are global operations and affect the ordering of other atomic operations in the thread that executed the fence."_ – pg. 166

> _"Fences restrict this freedom \[`std::memory_order_relaxed`\] and introduce happens-before and synchronises-with relationships that weren’t present before."_ – pg. 166

[fence.cpp](fence.cpp)

Based on the previous example, it can be seen that fences are a way to bring about _"store-release, load-acquire"_ semantics amidst `std::memory_order_relaxed` operations.

If we were to shift the `std::atomic_thread_fence()` up one level, then the two relaxed atomic bools would no longer be ordered, and cause our assert to fire.
```cpp
void write_ab1_then_ab2() {
    std::atomic_thread_fence(std::memory_order_release);
    ab1.store(true, std::memory_order_relaxed);
    ab2.store(true, std::memory_order_relaxed);
}
```

#
### The real benefit...
> _"...of using atomic operations to enforce an ordering is that they can enforce an ordering on non-atomic oper- ations and avoid the undefined behavior of a data race, ..."_ – pg. 168

[non_atomic.cpp](non_atomic.cpp)

> _"...the fences enforce an ordering on the operations on x, once the reading thread has seen the stored value of y. This enforced ordering means that there’s no data race on x, even though it’s modified by one thread and read by another."_ – pg. 169

#
### Ordering of non-atomic operations
> _"Ordering of non-atomic operations through the use of atomic operations is where the sequenced-before part of happens-before becomes so important."_ – pg. 169

> _"If a non-atomic operation is sequenced before an atomic operation, and that atomic operation happens before an operation in another thread, the non-atomic operation also happens before that operation in the other thread."_ – pg. 169

> _"Although other mutex implementations will have different internal operations, the basic principle is the same: lock() is an acquire operation on an internal memory location, and unlock() is a release operation on that same memory location."_ – pg. 170

#
### Summary
A cool chapter, in all fairness.

For one, the examples were (for the most part) fully-functional, which has helped a lot with one, grasping the concepts, and two, making progress through the book instead of spending time on making the examples functional!

I still feel like more contextual examples could be given, combined with less waffle, in order to get the points across, as (despite the content) I still feel like there's a lot more to be learnt about the different semantics and concepts like _happens-before_ and syncrhonises-with_.

It will be interesting to see the higher-level synchronisation facilities used in combination with atomics to create efficient containers and algorithms.

#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
