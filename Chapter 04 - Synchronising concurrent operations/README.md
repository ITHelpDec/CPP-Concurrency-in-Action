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
### Returning results from background tasks
`std::thread` doesn't do this, but we can call on `std::async` to help call an asychronous task for which we don't need the result right away.

`std::async` returns a `std::future - _when you need the value, you just call `.get()` on the `std::future`, and the thread blocks until the future is ready and then returns the value._

[future.cpp](future.cpp)

We also learn a bit more about `std::async`, although the layout to listing 4.7 in the book is keek - global variables everywhere and no structure, so have tweaked and attached below.

[async.cpp](async.cpp)
#
### `std::packaged_task<T>`
More half-baked examples - you'd be better off looking at use of `std::packaged_task` from C++ High Performance ([here](https://github.com/ITHelpDec/CPP-High-Performance/blob/2e61864d92c2981af90dfb536b6b318e18746e36/Chapter%2011%20-%20Concurrency/task.cpp)), or from cppreference ([here](https://en.cppreference.com/w/cpp/thread/packaged_task)).

[half_packaged_task.cpp](half_packaged_task.cpp)

> _"What about those tasks that can’t be expressed as a simple function call or those tasks where the result may come from more than one place?..."_ – pg. 87

#
### Introducing - `std::promise`
I've had a look at promises towards the [latter chapters of in C++ High Performance](https://github.com/ITHelpDec/CPP-High-Performance/search?q=promise), but a similar example of their use and benefit is provided in regards to an application that handles multiple connections - threads are finite and resource-heavy, so the author says it's commonplace for typically one thread to be in charge of connections, taking advantage of `promise / future` pairs to handle the connections.

Predictably, this is demonstrated with more pseudo-code.

[pseudo_promise.cpp](psuedo_promise.cpp)

> _"`std::promise<T>` provides a means of setting a value (of type T) that can later be read through an associated `std::future<T>` object"_ – pg. 87

I think a better introductory code sample to `std::promise` would be something like that found on [cplusplus.com](https://cplusplus.com/reference/future/promise/), although it appears to be a little different than most of the other examples that call `std::move` on the future instead of passing by reference...

<details>

<summary><b><code>std::promise</code></b> <i>(click to expand / collapse)</i></summary>

```c++
#include <future>
#include <iostream>

void print_int(std::future<int> &fut) {
    int x = fut.get();
    std::cout << "value: " << x << '\n';
}

int main()
{
    std::promise<int> p;                        // create a promise
    
    std::future<int> fut = p.get_future();      // assign future from promise
    
    std::thread t(print_int, std::ref(fut));    // send future to a new thread
    
    p.set_value(69);                            // fulfill promise (syncs with getting future)
    
    t.join();                                   // join back up to main()
    
    return 0;
}

```

</details>

#
### Exceptions
> _"...[if the function call invoked as part of `std::async` throws an exception], <b>that exception is stored in the `std::future` in place of a stored value.</b>"_ – pg. 89

The same thing applies to `std::packaged_task` and `std::promise`, just with `std::promise` we would need to call `.set_exception()` as part of a `try {..} catch {...}` block instead of `.set_value()` - e.g. ...

```c++
std::promise<int> p;

try {
    p.set_value(69);
} catch (...) {
    p.set_exception(std::current_exception());
}
```
...but you can also use...
```c++
p.set_exception(std::make_exception_ptr(std::logic_error("woof woof")));
```
...if you know the exception ahead of time to store the exception directly instead of throwing, which is, arguably, a little cleaner than using a try / catch (it also appears to provide the compiler with a greater opportunity to optimise the code further down the line).

If we destroy the `std::promise` or `std::packaged_task` associated with a `std::future` before it is "ready" then you will store a `std::future_error` exception with the error code `std::future_error::broken_promise.

> _"...by creating a future, you make a promise to provide a value or exception, and by destroying the source of that value or exception without providing one, you break that promise."_ – pg. 90

#
### `std::shared_future`
`std::future` has its limitations e.g. only one thread can wait for the result - this is where a `std::shared_future` kicks in.

> _"If you access a single `std::future¦ object from multiple threads without additional synchronisation, you have a data race and undefined behavior."_ – pg. 90

This is by design (models unique ownership / after the first call to `.get()` there is no value left to retrieve), but where `std::future` instances are only <ins>_moveable_</ins>, `std::shared_future` instances are <ins>_copyable_</ins> 😊

The preferred way to use `std::shared_future` is to pass a copy to each thread - it is also recommended to remember to protect multiple accesses to a single object with locks to avoid data races.

We can transfer ownership of a `std::future` to a `std::shared_future` with `std::move`:
<details open>
<summary><b>Transferring ownership</b> <i>(click to collapse / expand)</i></summary>

```c++
#include <future>

int main()
{
    std::promise<int> p;
    std::future<int> fut = p.get_future();
    
    assert(fut.valid());
    
    std::shared_future<int> sf = std::move(fut);
    
    // or, implicit construction from an rvalue
    // std::shared_future<int> sf(p.get_future()); 
    
    assert(!fut.valid());
    assert(sf.valid());
    
    return 0;
}
```
</details>

As a side note, from a degbugging point of view, `assert` seems to be more useful than I originally thought, providing syslog-like messages (function, file, line, etc., ...) without having to construct them manually:
```powershell
Assertion failed: (!sf.valid()), function main, file main.cpp, line 248.
```

#
### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
