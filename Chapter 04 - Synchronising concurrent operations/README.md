# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 04 - "Synchronising concurrent operations"

### Intro
Succinct introduction to this chapter...
> _"In this chapter, I’ll discuss how to wait for events with condition variables, futures, latches, and barriers, and how to use them to simplify the synchronisation of operations."_ – pg. 73

...followed by another nice analogy involving overnight trains - stay awake? Set an alarm? Or have someone wake you when you reach your destination?

I know which option I'd choose! 😃

#
### Waiting for an event or other condition
You could just lock a mutex and keep checking for a flag to update, but it's wasteful (and explained very well in the book).

Another way is to send the thread to sleep for set intervals between the checks (not perfect, but better) - below is an adaptation of the example from the book that shows this sleep-wait in action (you can all probably guess by now the quality of the example...)

[sleep.cpp](sleep.cpp)

A more idiomatic approach, however, would be to use a "condition variable" - threads can then notify one or multiple other threads waiting on this condition variable to be true before continuing on with their work.

#
### Condition variables
We have two on offer:
* `std::conditioin_variable` (uses a `std::mutex` - preferred)
* `std::condition_variable_any` (use anything that fulfills `std::mutex` criteria, but is a little more resource heavy as a result of the flexibilty).

The next example is the worst example out of the whole book so far...it shows one function with `std::lock_guard` and `notify_one()`, and another with `std::unique_lock` and `wait()` with a lambda.

I'll be honest - it's shit; it's shit and it's lazy (even the expanded [source file](https://github.com/anthonywilliams/ccia_code_samples/blob/6e7ae1d66dbd2e8f1ad18a5cf5c6d25a37b92388/listings/listing_4.1.cpp) is lazy).

You'd be better off looking at the explanation on [cppreference](https://en.cppreference.com/w/cpp/thread/condition_variable) (included below with an extra sleep).

[condition_variable.cpp](condition_variable.cpp) | [cppref_cv.cpp](cppref_cv.cpp)

> _"This is why you need the std::unique_lock rather than the std::lock_guard — the waiting thread must unlock the mutex while it’s waiting and lock it again afterward, and std::lock_guard doesn’t provide that flexibility."_ – pg. 76

The term "spurious wake" was used at least once in C++ High Performance, and Concurrency in Action was referenced for learning more about this term - below is our first encounter of this term (relating to the most recent piece of code from the author).

> _"When the waiting thread reacquires the mutex and checks the condition, if it isn’t in direct response to a notification from another thread, it’s called a spurious wake"_ – pg. 76

As mentioned earlier in this book, holding a mutex for longer than needed is less than ideal - the author recommends using a queue to transfer data between threads ("done well, ...\[it\] greatly reduces the possible number of synchronisation issues and race conditions.").

#
### Building a thread-safe queue
Starting with the standard library, and progressing to a thread-safe version.

[std_queue.cpp](std_queue.cpp) | [ts_queue_draft.cpp](ts_queue_draft.cpp) | [ts_queue_full.cpp](ts_queue_full.cpp)

Some useful takeaways from this exercise were:
* how to build a class with allocators (like we did with `ts::stack`)
* the use of `this` within a `std::condition_variable::wait` capture list when dealing with classes
* lambda notation over function pointers / casting to avoid member-function overload ambiguity when using threads

A suggestion from the author is that if the waiting thread is going to wait only once for a condition to return `true` (i.e. a one-off), then a `std::condition_variable` might not be the best tool for the job - `std::future` might be more appropriate.

#
### `std::future`
The examples might be undercooked, but the analogies are solid; this time round the author compares `std::future` to waiting on your flight to Tenerife - the only thing you're waiting for is the announcement to say your flight is boarding at gate 34A (and that flight will only happen once, you can't really miss it).

> _"Once an event has happened (and the future has become ready), the future can’t be reset."_ – pg. 81

We have two instances of futures:
* `std::future<T>` (unique futures - modelled off of `std::unique_ptr<T>)
* `std::shared_future<T>` (shared futures - modelled off of `std::shared_ptr<T>)

There are also `void` specialisations that should be used when there's no associated data (`std::future<void>` / `std::shared_future<void>`).

There are allllso experimental versions of these futures (experimental in syntax and semantics, as they might appear differently in an upcoming release, rather than the quality of code).

#
### Returnign results from background tasks
`std::thread` doesn't do this, but we can call on `std::async` to help call an asychronous task for which we don't need the result right away.

`std::async` returns a `std::future - _when you need the value, you just call `.get()` on the `std::future`, and the thread blocks until the future is ready and then returns the value._

[future.cpp](future.cpp)

#
### ...work in progress
