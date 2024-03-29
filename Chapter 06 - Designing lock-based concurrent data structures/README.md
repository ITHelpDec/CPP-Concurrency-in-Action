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

#
### Caveats with this linked list
> _"The most obvious problem is that `.push()` can modify both head_ and tail_, so it would have to lock both mutexes."_ – pg. 184

> _"The critical problem is that both `.push()` and `.pop()` access the next pointer of a node where there is only one item, `head_ == tail_` (.:. `head_->next_` == `tail_->next_`) – pg. 184

#
### Enabling concurrency by separating data
> _"You can solve this problem by preallocating a dummy node.."_ – pg. 185

> _"If you add a node to the queue (so there’s one real node), then `head_` and `tail_` now point to separate nodes, so there’s no race on `head_->next_` and `tail_->next_`..."_ – pg. 185

[dummy_node.cpp](dummy_node.cpp)

I tweaked a few bits from the example to avoid calls to `operator new` and opt for perfect forwarding instead of passing by value.

With these amendments, `.push()` now only accesses `tail_`; `.try_pop()` accesses both, but the lock is short-lived.

> _"The big gain is that the dummy node means `.try_pop()` and `.push()` are never operating on the same node, so you no longer need an overarching mutex."_ – pg. 186

#
### Strategy
> _"...hold the locks for the least amount of time possible."_ – pg. 186

* `.push()` - lock all accesses to `tail_`
* `.try_pop()` - be frugal with locks on `head_` and `tail_`

We can then pop these requirements into private member functions to take care of the donkey work.

[fine_grain_queue.cpp](fine_grain_queue.cpp)

This has been the most enjoyable part of the book so far - placing the odd mutex is nothing out of this world, but it's nice to finally see something that actually looks like legit multi-threading!

I created a set of threads and futures and a container for the results - it would be interesting to see how this would compare to a single-threaded approach in terms of speed.

> _"Even though each member function holds a lock on a mutex, they hold locks on different mutexes, ..."_ – pg. 188

> _"Because the call to `.get_tail()` locks the same mutex as the call to `.push()`, there’s a defined order between the two calls."_ – pg. 188

> _"It’s also important that the call to `.get_tail()` occurs inside the lock on head_mutex. If it didn’t, the call to `.pop_head()` could be stuck in between the call to `.get_tail()` and the lock on the head_mutex"_ – pg. 188

https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/424b817e7c3d9eb01447d34a135e88c6cba9b226/Chapter%2006%20-%20Designing%20lock-based%20concurrent%20data%20structures/broken_pop_head.cpp#L3-L16

Something like this could destroy the data structure, by moving `head_` beyond `tail_` and off the end of the list.

Keeping the call to `.get_tail()` inside the lock on the `head_` mutex _"ensures that no other threads can change `head_`, and `tail_` only ever moves further away...head can never pass the value returned from `.get_tail()`, so the invariants are upheld."_ – pg. 189

#
### Exceptions
* `.try_pop()`? Only the mutex locks, therefore exception-safe.
* `.push()`? All heap allocations (at least in my tweaks) are smart pointers, and similar mutex lock scenario, so also exception-safe.

> _"only one thread can call `.pop_head()` at a time, but multiple threads can then delete their old nodes and return the data safely."_ – 190

#
### Wait and pop
> _"...if the copy assignment throws an exception (as it might), the data item is lost..."_

[final_queue.cpp](final_queue.cpp)

This is quite impressive, and really starting to grow.

I've made some amendments based on previous recommendations in the book, such as mutable mutexes for a const member function, `.empty()` (we needed to mark `.get_tail()` as const, too), and removing the `std::move` from our return value (otherwise bye-bye, copy elision!).

I've also included a `main()` function to test of all of the member functions in our finished class.

I considered putting up a PR to address a few of the changes mentioned above, but there isn't one fully-fledged implementation of this _"unbounded"_ queue (only bits), so I'll leave it be for now.

#
### Unbounded
Our queue is "unbounded" because values canbe added until we run out of memory; a "bounded" container would have a set limit of how many elements it can hold, which makes it a good way of ensuring an even spread of work across threads.

We could change the condition variable in `.push(V &&val)` to wait for the queue to have fewer than a set maximum number of items if we wanted to make our current queue bounded.

#
### auto it == problem!
Handling iterators in a multithreaded container is a tricky proposition.

> _"Correctly handling iterators requires you to deal with issues such as another thread deleting the element that the iterator is referring to, which can get rather involved"_ – pg. 195

#
### Threadsafe associative container
Our threadsafe associative container will do four things:
* Add a new key / value pair
* Change associated value
* Remove a key / value pair
* Obtain associated value from key

Remember the principles, like not returning references and applying mutex locks around the entirety of each member function to protect the underlying data structure (although it is a little restrictive).

> _"The biggest potential for a race condition is when a new key/value pair is being added; if two threads add a new value, only one will be first, and the second will therefore fail."_ – pg. 195

#
### Understanding the container
There are three common ways of implementing an associative container
* A binary tree, such as a _red-black tree_ (`std::map<K, V>`)
* A sorted array (`std::set<T, B>`?)
* A hash table (`std::unordered_map<K, V>`)

> _"Assuming a fixed number of buckets, which bucket a key belongs to is purely a property of the key and its hash function. This means you can safely have a separate lock per bucket."_ – pg. 196

[ts_map.cpp](ts_map.cpp)

We have yet another piece of code that doesn't run out of the book - I've made a few functional / stylistic tweaks in my example, but I've also raised a PR to address the compilation error(s) [here](https://github.com/anthonywilliams/ccia_code_samples/pull/34) in the hope of helping any future readers encountering a similar problem.

The test within `main()` isn't the best, as it seems the single-threaded approach is faster than our multi0threaded container - I'm open to any suggestions to make better use of the member functions and hardware.

> _"Because the number of buckets is fixed, the `.get_bucket()` function can be called without any locking, and then the bucket mutex can be locked either for shared (read-only) ownership, or unique (read/write) ownership, ..."_ – pg. 198

#
### Exceptions
* `.value_for(...)` - no exception, no cry
* `.remove_mapping(...)` - `.erase()` won't throw, so all good 👍🏻
* `.add_or_update_mapping(...)` - `.push_back()` is fine, replacing a value might throw

#
### Snapshots
We're back to half-baked examples that don't compile again with this one...the first issue being that (provided we want this to be a public member function in our map) the `bucket_iterator` is a private `typedef` located in `bucket_type`, not our map - thankfully, say "hello" to our little friend, `auto` 😊

The second issue is that our data (`data_`) and mutexes (`sm_`) are also private members in a sub-class, therefore `.get_map()` must be declared as a `friend` within `bucket_type` if we hope to gain access.

[get_map.cpp](get_map.cpp)

[Never had to raise so many PR's in my life!](https://github.com/anthonywilliams/ccia_code_samples/pull/35) 😅

#
### Writing a thread-safe list
#### Iterators
> _"The alternative is to provide iteration functions such as `std::for_each` as part of the container itself. This puts the container squarely in charge of the iteration and locking, but it does fall foul of the deadlock avoidance guidelines from chapter 3."_ – pg. 200

#### Expectations
* Add an item
* Remove an item
* Find an item
* Update an item
* Copy an item

> _"The basic idea with fine-grained locking for a linked list is to have one mutex per node."_ – pg. 200

[linked_list.cpp](linked_list.cpp)

This was a more impressive example!

The last map was slower than a sequential map, but this list seems very quick - I was able to produce 1000's of answers in the blink of an eye while the other threads were doing their thing (populating, modifying and erasing).

> _"Also, the slow memory allocation happens outside the lock, so the lock is only protecting the update of a couple of pointer values that can’t fail."_ – pg. 202

Provided the predicates and functions are well-behaved there should be no change of deadlocks in this example.

> _"The only potential candidate for a race condition is the deletion of the removed node in `.remove_if()`...(it’s undefined behavior to destroy a locked mutex)."_ – pg. 203
 
#
### Summary
This has been a really insightful chapter.

Far more examples of code that actually work (bar the odd one), but it's been a great insight into creating concurrenct data structures, as well as a great refresher of linked-lists.

One really nice takeaway was seeing the use of smart pointers to safely allocate memory on the heap - typically you would see this done on leetcode with raw calls to `operator new`, so I would like to go back through the basic linked-list questions to re-learn the fundamentals (I'll either dedicate a week or so to that before I get into the next chapter or do one a day - we'll see!).

Now on to lock-free containers!

#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
