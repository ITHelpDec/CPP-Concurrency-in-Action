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

#### ADDENDUM
Whilst `std::atomic<std::shared_ptr<T>>` might not be a fully-accepted thing yet, `std::atomic_load(std::shared_ptr<T>)` looks to be valid from at least C++11 according to Fedor Pikus' talk at [27:28](https://youtu.be/9hJkWwHDDxs?t=27m28s) (definitely worth a watch).

#
### Push with diagrams
Good old mermaid to the rescue!

```mermaid
flowchart LR
    1 ~~~ head_
    head_((head_)) ==> next_((head_->next_))
```

```mermaid
flowchart LR
    2 ~~~ new_node((new_node))
    new_node ~~~ head_((head_))
    head_ ==> next_((head_->next_))
```

```mermaid
flowchart LR
    3 ~~~ new_node((new_node))
    new_node ==> head_((head_\n=\nnew_node->next_))
    head_ ==> next_((new_node->next_->next_))
```

```mermaid
flowchart LR
    4 ~~~ new_node((new_node\n=\nhead_))
    new_node ==> head_((head_->next_))
    head_ ==> next_((head_->next_->next_))
```

#
### `compare_exchange_weak(...)`
This is a pretty cool feature of `compare_exchange_weak()`:
> _"...if it returns false to indicate that the comparison failed (for example, because `head_` was modified by another thread), the value supplied as the first parameter (`new_node->next_`) is updated to the current value of `head_`."_ – pg. 211

> _"Because you build the data to be stored as part of the node, and you use `.compare_exchange_weak()` to update the `head_` pointer, there are no problematic race conditions here."_

#
### Leaky pop
1) Read the current value of head
2) Read `head_->next_`
3) Set `head_` to `head_->next_`
4) Return the data from the retrieved node
5) Delete the retrieved node

The tricky part is if one thread is at step 5 before the other gets to step 2 and tries to dereference a dangling pointer - we'll leave step 5 for later on.

Another doozy is if two threads read the same value of `head_` they will return the same node - `.compare_exchange_weak()` comes to the rescue again, failing when either a new node has been pushed or another thread has popped the node you were trying to pop.

[leaky_stack.cpp](leaky_stack.cpp)

There are a few more gotchas with this example.

Firstly, I don't agree with the following statement:
> _"...if `head_` is a null pointer, it will cause undefined behavior as it tries to read the next pointer. **<ins>This is easily fixed by checking for `nullptr` in the while loop</ins>**"_ – pg. 212

It doesn't matter if I write `if (!head_)`, `if (!old_head)` or `if (!old_head->next_)` in the body of the loop - all of these checks will throw for bad access if we encounter a race condition, although a few pages further on we see the other include a node check inside the condition check of the while statement, not the body of the loop.

This still doesn't work for our `void pop(T &val)` function, unfortunately - I have only managed to get it to work when checking for an empty node at the beginning of the function.

Typically, it is more common to check if a container is empty at the _**start**_ of the operation (i.e. before we perform any operations on a `nullptr`).

My one question here is - do we perform the same
...
```cpp
if (!head_) { return std::shared_ptr<T>(); }
```
...check (like we do in our other `.pop()` function), or do we follow the example? I almost want to side with the early-exit check, because the likes of `.compare_exchange_weak()` will only return false when someone else has beat you to the chase.

I know I'm making a habit of this, but I've raised a PR [here](https://github.com/anthonywilliams/ccia_code_samples/pull/36) with a better rationale for anyone that might be interested - given the inaction of the other PR's, there's a high chance nothing will happen, but hopefully future readers might find it beneficial (myself included, especially if I find out I've made a mistake!).

I also came up with (what I hope) is an atomic way of printing our container (using `.pop()` as inspiration), although, again, I wonder if it's even necessary, so feedback is welcome.

https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/9e0806110e42c5f78ab325c620edc5688aa3b1ba/Chapter%2007%20-%20Designing%20lock-free%20concurrent%20data%20structures/leakier_stack.cpp#L33-L40

[leakier_stack.cpp](leakier_stack.cpp)

> _"Note that although this is lock-free, it’s not wait-free, because the while loops in both `.push()` and `.pop()` could in theory loop forever if the `.compare_exchange_weak()` keeps failing."_

#
### Fixing the leaks
> _"If there are no threads calling `.pop()`, it’s perfectly safe to delete all the nodes currently awaiting deletion."_ – pg. 214

First listing in the book only provided a declaration (nearly sent my bloody boiling...), but then goes on to provide a full breakdown of what `.try_reclaim()` does.

[reclaimable_stack.cpp](reclaimable_stack.cpp)

An important part of this code is claiming exclusivity of the list with `.exchange()` - the, if `--threads_in_pop` returns `0`, we know that no other thread can be accessing this list of pending nodes (there might be new ones, but we're not fussed on them now).

#
### Timing is important
> _"The longer the time between when the thread first finds `threads_in_pop` to be equal to 1 and the attempt to delete the nodes, the more chance there is that another thread has called `.pop()`, and that `threads_in_pop` is no longer equal to 1, preventing the nodes from being deleted."_ – pg. 218

#
### The Essence of Quiescence
> _"The key is to identify when no more threads are accessing a particular node so that it can be reclaimed. By far the easiest such mechanism to reason about is the use of hazard pointers."_ – pg. 218

#
### Hazard Pointers
> _"They are so called because deleting a node that might still be referenced by other threads is hazardous."_ – pg. 218

> _"When a thread wants to delete an object, it must first check the hazard pointers belonging to the other threads in the system. If none of the hazard pointers reference the object, it can safely be deleted."_ – pg. 218

A thing to note is that using `.compare_exchange_weak()` in place of `.compare_exchange_strong()` in our example _"would result in resetting our hazard pointer unnecessarily."_ (pg 220)

A quick side note to say that this example is getting larger and larger with very litte end in sight. We seem tp ne adding unfinished functions to other new functions )op top of the alread-existing functions), _then_ defining them as we introduce another function in a similar pattern - it's making this whole thing a little hard to follow, and now I just want to see the back of the chapter...

#
### `.compare_exchange_strong()`

> _"If the `.compare_exchange_strong()` fails, another thread owns that entry, so you move on to the next."_ – pg. 222</br></br>_"If the exchange succeeds, you’ve successfully claimed the entry for the current thread, so you store it and stop the search"_ – pg. 222</br></br>_"If you get to the end of the list without finding a free entry, there are too many threads using hazard pointers, so you throw an exception."_ – pg. 222

#
### Cache Rich
> _"Once the `hp_owner` instance has been created for a given thread, further accesses are much faster because the pointer is cached, so the table doesn’t have to be scanned again."_ – pg. 222

#
### Never-ending story
And it keeps going....now we're writing out the reclaim functions...then even more stuff...I still don't think I have a fully-functioning programme to demonstrate after reading it all.

Not really sure how to react to this chapter - we're going downhill again with feature-dumping and waffle. The lack of whitespace is also noticeable in the these listings.

Short, purposeful commits...and now we get to the end of the paragraph and the summary is atomic operations like this can be up to 100x slower than standard operations...

No bloody wonder!

Now we're talking about patents for hazard pointers that were abandoned by IBM in 2010...

12 pages of waffle on reclamation and I feel like I have very little to show for it...this chapter has taken such a nosedive...

[hazardly-explained_oointers.cpp](hazardly-explained_oointers.cpp)

#
### Better reclamation strategies
> _"Rather than checking around `max_hazard_pointers` nodes every call to `.push()` (and not necessarily reclaiming any), you’re checking 2 * max_hazard_pointers nodes every max_hazard_pointers calls to pop() and reclaiming at least max_hazard_ pointers nodes."_

#
### Reclamation through reference-counting
> _"If your platform supplies an implementation for which `std::atomic_is_lock_free(&some_shared_ptr<T>()` returns `true`, the whole memory reclamation issue goes away."_

This is what I was trying to do earlier on in my write-up - use smart pointers over their raw equivalent to avoid raw calls to `operator new`.
```cpp
std::shared_ptr<int> sp;
std::cout << "atomic sp supported?: " << std::atomic_is_lock_free(&sp) << '\n';
// atomic sp supported?: false
```
Sadly no love for Apple Clang...[Godbolt works, though](https://godbolt.org/z/TTW1TjWhh)!

[ref-clamation.cpp](ref-clamation.cpp)

Having mentioned the lack of love, there are no issues with the compilation or running of our above example, and actually this has been one of the most insightful examples yet.

An even more curious part is that the "early exit" `if (!head_) return {};` approach I mentioned earlier in the write-up might not be a good idea after all.

Having encountered runtime errors with the "early-exit" approach, I'm thinking that even _if_ `head_` is valid when we perform our check, it may not be valid at the point we call `std::atomic_load(&head_)`, so if we don't include `original_head` in our `while` conditions, we run the risk of performing operations on a `nullptr` - now I understand why the author wrote their functions as they did.

The author says we can also use the experimental version of `std::atomic_smart_pointer`.

#
### Counted Node Pointer
Another half-baked example from the book calling a function that doesn't exist yet.

[half-baked.cpp](half-baked.cpp)

#
### Size matters
> _"On those platforms that support a double-word-compare-and-swap operation, this structure will be small enough for `std::atomic<counted_node_ptr>` to be lock-free."_ – pg. 230

> _"...`std::atomic<>` will use a mutex to guarantee atomicity when the type is too large for the platform’s atomic instructions \[DWORD CAS\] (rendering your “lock-free” algorithm lock-based after all)"_ – pg. 230

[slightly_more_baked.cpp](slightly_more_baked.cpp)

I've done some refactoring in `.push()` to turn 3 lines into 1, but – more alarmingly – it looks like there's a bug in this implementation, as popping from an empty stack (or even calling the destructor) causes `__atomic_add_fetch` to throw within `__libcpp_atomic_refcount_decrement`...after a bit of debugging, it appears the implicit boolean condition in our `if` statement indicates an empty stack one dereference too late.

I've attached a link to the PR [here](https://github.com/anthonywilliams/ccia_code_samples/pull/37), but the long and short of it is that we need to check if _`ptr_->next.ptr_`_ is a valid pointer, not `ptr_`.

£40-odd is a lot of money to pay for a second edition book with this many typos...

> _"...by incrementing the external reference count, you ensure that the pointer remains valid for the duration of your access."_ – pg. 231

> _"If the reference count is now zero, the previous value (which is what fetch_add returns) was the negative of what you added, in which case you can delete the node"_ – pg. 231

#
### Acquire-released bakes
Now that we have functioning code sample, we can start to relax some of the memory ordering to reduce any unnecessary overhead.

> _"...the `next_` value is a plain non-atomic object, so in order to read this safely, there must be a "happens-before" relationship between the `.store()`\* (by the pushing thread) and the `.load()`\*\* (by the popping thread)"_ – pg. 232

\* _(...you need a release operation \[or stronger\] to get a happens-before relationship between threads.)_

\*\* _(...you must have an operation that’s `std::memory_order_acquire` or stronger before the access to `next_`.)_

> _"...you need to ensure that `.swap()` happens before the `delete` in order to avoid a data race.</br></br>The easy way to do this is to make the fetch_add() in the successful-return branch use std::memory_ order_release and the fetch_add() in the loop-again branch use std::memory_order _acquire."_ – pg. 234

[ordered_bake.cpp](ordered_bake.cpp)

The following programme works (with our amended if statement condition), but not consistently - every so often we come up against a bad access, so somekind of race condition must be happening (there's also mention of a mutex lock in the assembly of this 2013 MacBook Pro?).

Saying that, with thread sanatiser running, it doesn't seem to be highlighting any race conditions.

#
### Lock-free queue
Similar kind of setup, only this time round we follow a _FIFO_ pattern (First-in, First-Out), instead of the stack's _FILO_ pattern.

[lf_queue.cpp](lf_queue.cpp)

It's worth noting, however, that this implenetation will result in race conditions when either `.push()` or `.pop()` are called concurrently.

> _"both \[calls to `.push()`\] read the same value for `tail_`, and consequently both update the data members of the same node when setting the `data_` and `next_` pointers – this is a data race!"_ – pg. 238

I'm going to see if the following statement applies with the succinct `std::shared_ptr<>` method used earlier.

[ref-clamation_queue.cpp](ref-clamation_queue.cpp)

> _", ...in order to get the required happens-before relationship between `.push()` and `.pop()`, you need to set the data items on the dummy node before you update `tail_` (although this leads to data races with concurrent calls to `.push()` because theyve read the same `tail_` pointer."_

#
### "Single-producer, single-consumer"
Whilst a mouthful, this is quite a powerful statement in terms of synchronisation.

> _"The important thing in that case is the happens-before relationship between the `.push()` and the `.pop()`"...</br></br>"...the store to `tail_` synchronises with the `.load()` from `tail_`..."</br>"...the `.store()` to the preceding node’s `data_` pointer sequenced before the `.store()` to `tail_`..."</br>"...and the `.load()` from `tail_` is sequenced before the `.load()` from the `data_` pointer..."</br>"..., so the `.store()` to `data_` happens before the `.load()`, and everything is OK.</br></br>This is therefore a perfectly-serviceable single-producer, single-consumer (SPSC) queue."_ – pg. 237-238

#
### In-between dummies
Adding a dummy node between the real nodes allows us to make `next_` atomic - this would lead to twice as many memory allocations, though (pg. 238).

Another option is to make `data_` atomic, and if `std::shared_ptr<>` is lock-free then job's a good'un. Otherwise, we can return `.pop()` as a `std::unique_ptr<>` and store the `data_` as a plain pointer in the queue (thus allowing for `std::atomic<T*>` and `.compare_exchange_strong()`).

[broken_push.cpp](broken_push.cpp)

This only fixes one race, unfortunately, as if another thread updates and deallocates a node before we perform a dereferenxe then we end up with undefined behaviour.

Looks like the "internal & external counters" approach is recommended again (a technique the author first encountered in Joe Seigh's work, "Atomic Ptr Plus").

[pointer_plus.cpp](pointer_plus.cpp)

This example is half-baked again though, with three functions not even declared, and even when they are, the programme crashes - we'll see if the gaps are filled later in the chapter.

The bit field approach is interesting, aiming to keep the total counter size to 32 bits to fit inside a typical DWORD for 32- and 64-bit machines.

https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/2251e079c70287a16e91ea4d076c5c3b90d04922/Chapter%2007%20-%20Designing%20lock-free%20concurrent%20data%20structures/pointer_plus.cpp#L48-L51

> _"It’s important to update these counts together as a single entity in order to avoid race conditions, ..."_ – pg. 241

There must be a better way to word this...still not really sure what it's trying to get at.
> _"The node is initialised with the `internal_count_` set to `0` and the `external_counters_` set to `2`, because every new node starts out referenced from `tail_` and from the `next_` pointer of the previous node once you’ve added it to the queue"_ – pg. 241

We now add the pop feature, but it still doesn't work, so we'll see how things progress with the count implementations.

[pointer_plus_pop.cpp](pointer_plus_pop.cpp)

I'm also unsure why the author chose `std::memory_order_acquire` for `.compare_exchange_strong()` when `std::memory_order_release` seems to be the norm with this kind of instruction. In fact, there are quite a few discussions [here](https://stackoverflow.com/questions/55704567/how-to-understand-calling-compare-exchange-strong-with-stdmemory-order-acquire) and [here](https://stackoverflow.com/questions/74127715/why-are-these-memory-orders-applied-here-in-the-implementation-of-the-lock-free) suggesting potential data races found using thread sanitiser.

https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/02d478ec8b3a334b7a006845f532c06a8e504930/Chapter%2007%20-%20Designing%20lock-free%20concurrent%20data%20structures/pointer_plus_pop.cpp#L95-L96

There's even a Godbolt link [here](https://godbolt.org/z/1n4f4ceTz) with the issues, and I've made the amendments suggested in the thread [here](https://godbolt.org/z/73TMd8Pnz) with `std::memory_order_acq_rel` - no data races.

The explanation from the stack exchange is actually really good.

> _"We read the object `*ptr` before the `.compare_exchange` (to get `ptr->count`).</br>But since the `.compare_exchange` is not `release`, the read of `*ptr` could in theory be reordered after it.</br>If so, then by the time the read of `*ptr` happens, the counter has already been decremented, and in the meantime some other thread might have decremented it again and then done `delete ptr`."_

The author of the comment then goes on to say:
> _"I think these can be fixed by upgrading the compare-exchanges to `acq_rel`.</br>That should ensure that all accesses to `*ptr` happen before its deletion. In fact, as you point out, we only need the `acquire` ordering in the case that the count has become zero, since it's only the `delete ptr` that must be protected from reordering before the `compare_exchange`. You could do that by having just `release` ordering on the `compare_exchange`, and putting an `acquire` fence inside the `if` block, just before `delete ptr`"_

https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/648f26dcc4656eba58ce70020c4b289e4d227a6a/Chapter%2007%20-%20Designing%20lock-free%20concurrent%20data%20structures/pointer_plus_pop.cpp#L95-L96

I've put up a PR [here](https://github.com/anthonywilliams/ccia_code_samples/pull/38) and linked back to the posts to give credit to the authors of the comments.

#
### Final product
Looks like we're at the finish line (I'll assume it works properly and maybe test later).

[pointer_plus_done.cpp](pointer_plus_done.cpp)

> _"If both the values are now zero, there are no more references to the node, so it can be safely deleted"_ – pg. 244

Yet again, though, this implementation is incomplete and isn't actually lock-free.

From a reader's point of view, I'm getting a little bored of the "here's how not to do it" examples - as interesting an exercise as it is, _**practice makes permanent**_, and so far 80-90% of the practice has been creating containers with data races and undefined behaviour. My argument would be that if we want to instill good practices from the get-go, then we should be spending 80-90% of our time writing code _**without**_ data races and undefined behaviour, then dedicate a little bit of time on how to break the rules.

I know we still have a few chapters left, but I also think it would be a good idea to dedicate a small portion of the book on how to use thread sanitiser to help spot or even visualise these kinds of data races.

#
### Helper threads
> _"..., you know exactly what needs to be done: the `next_` pointer on the `tail_` node needs to be set to a new dummy node, and then the `tail_` pointer itself must be updated."_ – pg. 244

> _"If you make the `next_` pointer in a node atomic, you can then use `.compare_exchange_strong()` to set the pointer.</br>Once the `next_` pointer is set, you can then use a `.compare_exchange_ weak()` loop to set the `tail_` while ensuring that it’s still referencing the same original node.</br>If it isn’t, someone else has updated it, and you can stop trying and loop again."_ – pg. 245

[listing_7-21.cpp](listing_7-21.cpp)

Thanks to [Nate Eldridge](https://stackoverflow.com/users/634919/nate-eldredge) for his Godbolt _producer / consumer_ layout - I used this in the last example, but found a peculiar error within Xcode regarding "nano zone" when trying to run the code with Thread Sanitiser on:
```bash
07 - Designing lock-free concurrent data structures(22505,0x1ec411e00) malloc:
nano zone abandoned due to inability to reserve vm space.
```
After a bit of research, a [very helpful person](https://shapeof.com/archives/2015/10/mallocnanozone.html) suggested modifying `MallocNanoZone` to `0` as part of our "Environment Variables" - a quick way to get there is to use `⌘` + `⇧` + `,` and then enter the following parameter:
```bash
MallocNanoZone=0
```
<img width="934" alt="malloc_nano_zone" src="https://github.com/ITHelpDec/CPP-Concurrency-in-Action/assets/34002836/ae3647bc-a9e6-4e22-bfc3-745ca1401c8b">

It also seemed important to clearly define our `node` constructor with `.store()` operations for the atomic member variables, otherwise Godbolt would hang and return a `143` error.

> _"In this example, you’re using the default `std::memory_order_seq_cst` ordering, so you could omit the explicit call to `.load()` and rely on the load in the implicit conversion to `counted_node_ptr`, but putting in the explicit call reminds you where to add the explicit memory ordering later."_ – pg. 245

#
### Faulty push
Again, to reiterate, our last `.push()` function was fualty, so the updated version is below:

[listing_7-22.cpp]([listing_7-22.cpp)

...[and it still doesn't work properly](https://godbolt.org/z/W8PMP67z4)...I'm really starting to lose the will with this book...

The constructor in `node` that fixed our issue before now causes an issue when we try to amend it with the new push function.

I've tried debugging, but not having any luck at all - if anyone has suggestions, feel free.

> _"Common techniques for optimizing memory allo- cation include having a separate memory allocator on each thread and using free lists to recycle nodes rather than returning them to the allocator."_ – pg. 247

#
### Guidelines for creating lock-free data structures
1) **_Use `std::memory_order_seq_cst` for prototyping_**
> _"In general, you can only determine which operations can be relaxed when you can see the full set of code that can operate on the guts of the data structure."_ – pg. 248

2) **_Use a lock-free memory reclamation scheme_**
> _"It’s essential to avoid deleting objects when other threads might still have references to them, but you still want to delete the object as soon as possible in order to avoid excessive mem- ory consumption."_ – pg. 248

We can do this by waiting for threads to stop accessing the container and deleting all elements pending deletion, using hazard pointers or reference counting.

> _"It’s much easier to write the algorithms if you know that the garbage collector will free the nodes when they’re no longer used, but not before."_ – pg. 248

We cam also recycle nodes, which helps with avoiding undefined behaviour (the memory never becomes invalid), but this can also lead to the [_ABA_ problem](https://www.infoq.com/news/2014/10/cpp-lock-free-programming/).

3) **_Avoid the ABA problem_**
> _"The most common way to avoid this problem is to include an ABA counter alongside the variable x."_ – pg. 249

> _"The ABA problem is particularly prevalent in algorithms that use free lists or other- wise recycle nodes rather than returning them to the allocator."_ – pg. 249

4) **Identify busy-wait loops and help the other thread_**
> _"If you end up with a busy-wait loop, you effectively have a blocking operation and might as well use mutexes and locks."_ – pg. 249

### Summary
This chapter had so much potential, but it was so poorly-put-together that I geneuinely couldn't wait to finish it

Whilst the beginning taught me a lot about using smart pointers and atomics to create lock-free (and sometimes wait-free) containers, I spent more time writing both incorrect and incomplete code that I don't think the author really properly tested.

This made the whole chapter feel more like marking someone's homework than actually learning about lock-free data structures.

I really like the concept of atomics and lock-free programming, through, and after watching quite a few talks from Herb Sutter and Fedor Pikus, I'd like to become more proficient with writing better lock-free / wait-free code.

Next up is worrying less about the synchronisation and more about the manipulation of data itself.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
