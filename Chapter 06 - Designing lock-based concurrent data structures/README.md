# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 06 - "Designing lock-based concurrent data structures"

### To lock or not to lock
That is the question.

We can either use the likes of an separate mutex and external locking to protect the data, or set about designing our own data structure for concurrent access (chapter 7 will touch on how to do this lock-free).

#
### Key points
* No data will be lost or corrupted
* All invariants will be upheld
* No problematic race conditions

#
### "Serialisation"
> _"A mutex protects a data structure by explicitly preventing true concurrent access to the data it protects."_ – pg. 174

> _"The smaller the protected region, the fewer operations are serialised, and the greater the potential for concurrency."_ – pg. 174

#
### Guidelines for design
* No thread can see invariants broken by other threads
* Provide complete operations instead of operation steps (helps avoid race conditinos)
* Be mindful of behaviour when exceptions are thrown
* Restrict scope of locks / use of nested locks to minimise chance of deadlocks

#
### Safe to access? (before or after)
> _"Generally, constructors and destructors require exclusive access to the data structure, ..."_ – pg. 175

Similar considerations should be given to the thread-safe intention of copy-/move-assignment, swap and copy / move constructors.

#
### Types of concurrency
> _"It’s not uncommon for data structures to allow concurrent access from multiple threads that merely read the data structure, whereas a thread that can modify the data structure must have exclusive access. (`std::shared_mutex`)"_ – pg. 175

> _"...it’s quite common for a data structure to support concurrent access from threads performing different operations while serializing threads that try to perform the same operation."_ pg. – 176

#
### Other golden nuggets
Plenty of golden nuggest so far in this chapter.
> _"You need to ensure that data can’t be accessed outside the protection of the mutex lock and that there are no race conditions inherent in the interface, ..."_ – pg. 176

# Thread-safe stack mk. II
[ts_stack.cpp](ts_stack.cpp)

We avoid a potential race condition by returning the popped value in `.pop()` (compared to how they're written in `std::stack<int>`).

Mutexes may throw, but it's rare; unlocking a mutex cannot fail, and `std::lock_guard` ensures the mutex is never left locked. – pg. 178

> _"The only member functions that aren’t safe are the constructors and destructors, but this isn’t a problem; the object can be constructed only once and destroyed only once."_ – ph. 178

> _"...because of the use of locks, only one thread is ever doing any work in the stack data structure at a time. This serialisation of threads can potentially limit the performance of an application..."_ – pg. 179

Some really good takeaways from this exercise, albeit not immediately evident from the example in the book - it's one of those things that only becomes evident in practice.

Firstly, lambdas make writing threads much easier, both in terms of reading and writing – testament to this is when passing an overloaded function like I tried to do (unsuccessfully) in Chapter 2. It is possible when using the old function-pointer-style method, but it requires `static_cast` and is very verbose when compared to the lambda.
https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/2e068cf7565349e8e226d2a8e8229cdd11dfa6b6/Chapter%2006%20-%20Designing%20lock-based%20concurrent%20data%20structures/ts_stack.cpp#L96-L99

The second takeaway is about ensuring we declare a `std::mutex` as `mutable` when it operates with a `const` function (like `bool empty() const`)

And speaking of const, I've also included a link that suggested veering away from returning variables marked as `const` - the article is only about a year old at the time of writing, so I'd be interested in higher others' thoughts.
https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/2e068cf7565349e8e226d2a8e8229cdd11dfa6b6/Chapter%2006%20-%20Designing%20lock-based%20concurrent%20data%20structures/ts_stack.cpp#L45-L59

#
### Threadsafe queue
[ts_queue.cpp](ts_queue.cpp)

Another helpful reminder to pair `std::unique_lock`'s with condition variables.

> _"...if more than one thread is waiting when an entry is pushed onto the queue, only one thread will be woken by the call to data_cond.notify_one()"_ – pg. 181

I experienced this once, before initialising a few elements sequentially to the queue – to avoid this, we can replace `.notify_one()` with `.notify_all()`.

Another suggestion is to _"move the `std::shared_ptr<T>` initialisation to the `.push()` call.

> _"Copying the std::shared_ptr<> out of the internal std::queue<> then can’t throw an exception, so wait_and_pop() is safe again."_ – pg. 181

[ts_queue_mkii.cpp](ts_queue_mkii.cpp)

> _"If the data is held by `std::shared_ptr<>`, there’s an additional benefit: the allocation of the new instance can now be done outside the lock in `.push()`"_ – pg. 182

This all seems a little expensive, regardless of whether or not the `std::shared_ptr<>` allocation is performed before the mutex begins, but maybe this is cost for exception safety?

#
### It's _inside_ the computer...
> _"In order to use finer-grained locking, you need to look inside the queue at its constituent parts and associate one mutex with each distinct data item."_ – pg. 183

![simple.gif](simple.gif)

#
### Linked lists (aka "queues")
[linked_list_queue.cpp](linked_list_queue.cpp)

This single-threaded linked-list implementation of a queue feels like a good old leetcode question, although this time it's nice to finally see an example being used with `std::unique_ptr<T>` in place of raw calls to `operator new` - I've made a few tweaks to the example, but I may revisit easy linked-list questions in the future and see if I can re-write them using smart pointers.

If I do then I'll attach them [here](https://github.com/search?q=repo%3AITHelpDec%2FLeetcode%20std%3A%3Aunique_ptr&type=code).

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
