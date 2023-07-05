# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 08 - "Designing concurrent code"

### To generalise or to specialise
> _"You need to decide whether to have “generalist” threads that do whatever work is necessary at any point in time or “specialist” threads that do one thing well, or some combination"_ – pg. 253

#
### Divide and conquer
> _"...the worker threads run these tasks independently, and the results are combined in a final reduction step"_ – pg. 253

> _"If you’re going to parallelise this algorithm, you need to make use of the recursive nature."_ – pg. 254

> _"By using `std::async()`, you ask the C++ Thread Library to decide when to run the task on a new thread and when to run it synchronously._ – pg. 245

#
### Start as we mean to go on...
I have spent far too much time debugging Listing 8.1 (issue and PR are [here](https://github.com/anthonywilliams/ccia_code_samples/issues/39) and [here](https://github.com/anthonywilliams/ccia_code_samples/pull/40)).

Combining Listing 6.1 and Listing 8.1 with the changes suggested above, we have a functioning piece of code that sorts a simple list, but it still seems to have issues every so often, especially with `-fsanitize=thread` enabled in Xcode.

[six_and_eight_sorter.cpp](six_and_eight_sorter.cpp) | [ts_stack_sorter.cpp](ts_stack_sorter.cpp)

A few things to bear in mind with these implementations is the importance of `std::move` when reading from teh top of our stack - without it, we will presented with an obscure compiler error that took far too long to debug.
```
No matching function for call to 'construct_at`
```
I've also rejigged the implementation from a `struct` to a `class`, keeping only `do_sort()` as a public member function; nothing else is called publicly - beyond the constructor and destructor, they're all just helper functions / private member variables).

We have similar issues using our lock-free "ref-clamation" stack from the previous chapter, but – again – this only works 8-9 times out of 10.

[lf_stack_sorter.cpp](lf_stack_sorter.cpp)

I spent far too long putting these examples together after the PR, but I wanted to make sure I was able to move on with functioning code samples.

---
On a personal note...

> _"As with most of the examples, this is intended to demonstrate an idea rather than being production-ready code.""

...from pg. 255 really isn't good enough - no book should be published with this many mistakes and bad practices under the guise of it not being "production-ready".

It's instructional, and as such should not only compile and run, but run as intended.

#
### Time to specialise
> _"An alternative to dividing the work is to make the threads specialists, where each per- forms a distinct task, just as plumbers and electricians perform distinct tasks when building a house."_ – pg. 258

> _"There are two big dangers with separating concerns with multiple threads. The first is that you’ll end up separating the wrong concerns."_ – pg. 259

> _"if two threads are communicating a lot with each other but much less with other threads, maybe they should be combined into a single thread."_ – pg. 259

#
### Pipelines

> _"If your task consists of applying the same sequence of operations to many independent data items, you can use a pipeline..."_ – pg. 259

There's a lot of waffle in these pages...

#
### `std::thread::hardware_concurrency()`
One reason to opt for `std::async` (or careful use of thread pools):
> _"Using `std::thread::hardware_concurrency()`...your code doesn’t take into account any of the other threads that are running on the system unless you explicitly share that information."_ – pg. 261

#
### Contention
> _"If two threads are executing concurrently on different processors and they’re both reading the same data, this usually won’t cause a problem; the data will be copied into their respective caches, and both processors can proceed."</br>"But if one of the threads modifies the data, this change then has to propagate to the cache on the other core, which takes time."_ – pg. 262

Blah, blah, blah, cores waiting on each other = contention, happens with mutexes, etc., etc., etc., ...

The author is suggesting a lot of "here's a thing, but here's a problem, and you probably won't resolve this problem, so you're stuck with it" - thing is, though, I don't want just problems; I want best practices.

#
### False sharing
Cache lines are (currently) typically 32 or 64 bytes wide.

Set of data access by thread on same cache line? Está bien.

Unrelated data items on cache lines to be accessed by different threads? No bueno.

Thread wants to changes value on cache line? Takes onwership, then transfers ownership to next thread.

> _"The cache line is shared, even though none of the data is, hence the term false sharing"_ – pg. 264

Solution? Put same thread items on same cache line.

We see reference to `std::hardware_destructive_interference_size` again (like from C++ High Performance [here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2011%20-%20Concurrency/README.md) and [here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2011%20-%20Concurrency/false_sharing.cpp)).

Again, more waffle...more threads than cores = _task switching_...increased pressure on cache...reference to the aforementioned's counterpart (`std::hardware_constructive_interference_size` - max number of consecutive bytes guaranteed to be on the same cache line).

We're like 10 pages in now - less talky, more code-y...

#
### Oversubscription
> _"Oversubscription can arise when you have a task that repeatedly spawns new threads without limits, .."_ – pg. 266

#
### "Designing data structures for multi-threaded performance"
This is addressed, but I'm  still not sure why this is appearing again, considering we've spent the last two chapter doing this with lock-based and lock-free approach.

Regardless, three things to bear in mind when designing our data structures are:
* contention
* false sharing
* data proximity

#
### Matrices
We have the classic matrix multiplication crux.

> _"...it’s clear that you ought to be accessing adjacent columns, so the N elements from each row are adjacent, and you minimize false sharing."_

We can invert the x by y approach to avoid cache-thrashing like we've done before ([here](https://github.com/ITHelpDec/CPP-Primer) and [here](https://github.com/ITHelpDec/CPP-High-Performance/blob/f54fe8caafddb709765e576b89b2d78bef14e3a3/Chapter%2004%20-%20Data%20Structures/loop_interchange.cpp#L19)), but I'm hoping we'll take a multi-threaded approach to this.

> _"If the space occupied by your N elements is an exact number of cache lines, there’ll be no false sharing because threads will be working on separate cache lines."_ – pg. 268

> _"On the other hand, if you have each thread compute a set of rows, ...it has a contiguous block of memory that’s not touched by any other thread."_ – pg. 268

> _"Dividing into rectangular blocks...has the same false-sharing potential as division by columns."_ – pg. 268

The example used is a 1,000 x 1,000 matrix (1,000,000 elements) across 100 processors:

10 rows each get us to 10,000, but to calculate those 10,000 results we need access to the entirety of the second matriax (another 1,000,000 elements) - total of 1,010,000 elements.

However...if we do 100 x 100 blocks (still 10,000 elements), the accesses are 100 from one matrix (100 x 1,000) and 100 from another.

This is 200,000 in total, and 200,000 vs 1,010,000 is a 5x reduction in reads.

Fewer reads means less likelihood of cache misses.

Great theory, but it would have nice to see actual code - again, "less talky, more codey".

> _"...look at all the aspects of the data access patterns carefully, and identify the potential causes of performance hits."_ – pg. 269
 
#
### Data access pattern considerations
* Ensure contiguous data is worked on by the same thread
* Minimise data required by any given thread
* Ensure data accessed by separate threads is sufficiently far apart to avoid false sharing (`std::hardware_destructive_interference_size` will help with this)

> _"...having data end up in different places on the heap isn’t a particular problem in itself, but it does mean that the processor has to keep more things in cache - this can be beneficial"_ – pg. 269

There's a good piece of info on using pointers in a tree with threads.

> _"If multiple threads need to traverse the tree, then they all need to access the tree nodes, but if the tree nodes only contain pointers to the real data held at the node, then the processor only has to load the data from memory if it’s needed."</br>"If the data is being modified by the threads that need it, this can avoid the performance hit of false sharing between the node data itself and the data that provides the tree structure."_ – pg. 269

To reiterate what the author mentioned, if we're using mutexes, then it's a good idea if both the mutexes and data are in close proximity to each other, although there can be a performance hit if other threads try to lock the mutex while it's held by the first thread.

#
### "Cushion for the pushin" (of improved mulithreading performance)
We can add large blocks of padding between mutexes and data members to test if either mutex contention of false sharing is affecting performance.
```cpp
template <typename T>
struct mutex_contention {
    std::mutex m_;
    char padding[std::hardware_destructive_interference_size];
    std::vector<T> data_;
};

template <typename T>
struct false_sharing {
    std::vector<T> v1;
    std::vector<T> v2;
    char padding[std::hardware_destructive_interference_size];
};
```
I found out the hard way back in C++ High Performance that we can also use `_X86_INSTRUCTION_STATE_CACHELINE_SIZE` on Intel Mac's where `std::hardware_destructive_interference_size` commnand isn't available.
```cpp
char padding[_X86_INSTRUCTION_STATE_CACHELINE_SIZE * 1024];
```

#
### Scaling
> _"Code is said to be scalable if the performance increases as more processing cores are added to the system"_ – pg. 270-271

#
### Exceptions
We can allow exceptions to propogate to the caller in sequential algorithms - we can't in parallel algorithms because it would be on the wrong call stack.

> _"If a function spawned on a new thread exits with an exception, the application is terminated."_ – pg. 271

We can see this with the `std::accumulate` example supplied in the book (although, yet again, [another PR](https://github.com/anthonywilliams/ccia_code_samples/pull/41) was needed for code that doesn't look like it was tested):

[par_accumulate.cpp](par_accumulate.cpp)

The first exception to watch out for is in the construction of our threads (same goes for `accumulate_block`:

> _"...the destructors of your new `std::thread` objects will call `std::terminate` and abort your programme."_ – pg. 272

We can use `std::packaged_task` to help handle our exception dilemma.

[packaged_accumulate.cpp](packaged_accumulate.cpp)

After rewriting and correcting this many examples, I don't know if can take this book seriously any more - the amount of mistakes is just mental...I've uploaded another PR [here](https://github.com/anthonywilliams/ccia_code_samples/pull/42) and decided to introduce some range-based `for` loops to sync our threads and futures (not sure why index-based for loops were still considered a good idea in 2017).

> _"You’re using std::packaged_task and std::future for the exception safety, ..."_ – pg. 274

> _"So, that’s removed one of the potential problems: exceptions thrown in the worker threads are rethrown in the main thread...you can use something like `std::nested_exception` to capture all the exceptions and throw that instead."_ – pg. 274

We can also use a `try / catch` to handle any exceptions between when we spawn the first thread and when we join them back to main (but it's ugly, expensive and repeats code).
```cpp
try {
    for (int i = 0; i != num_threads - 1; ++i)
        // ...
    __Tp last_result = accumulate_block<__ForwardIt,__Tp>()(block_start, last);
    for (auto &t : threads) { t.join(); }
} catch (...) {
    for (auto &t : threads)
        if (t.joinable()) { t. join(); }
    throw;
}
```
The author suggests wrapping the joins in a class as an idiomatic way of tidying resource (although, in hindsight, is this not what `std::jthread` does?).

[exception_safe_accumulate.cpp](exception_safe_accumulate.cpp)

#
### Avoiding leaked threads with `std::async`
> _"The key thing to note for exception safety is that if you destroy the future without waiting for it, the destructor will wait for the thread to complete."_

[async_accumulate.cpp](async_accumulate.cpp)

My one question was whether or not to pass the capture list by ref or value given the concurrent nature of the function - it looks like the function only reads (doesn't write), so it doesn't look like the preconditions are met to invoke a data race if we were to pass by reference (I hope) - feedback and corrections welcome.

#
### More scaling
> _"Threads often spend time waiting for each other or waiting for I/O operations to complete."_ – pg. 278

> _"Every time one thread has to wait for something (whatever that something is), unless there’s another thread ready to take its place on the processor, you have a processor sitting idle that could be doing useful work."_ – pg. 278

I suppose this is a reason why `std::this_thread::yield` is so important.

We have reference to Amdahl's law, which I covered in C++ High Performance [here](https://github.com/search?q=repo%3AITHelpDec%2FCPP-High-Performance%20amdahl&type=code); the same things apply - reduce "serial" components, reduce wait time and / or increase data provided to parallel sections.

$$ Overall\text{ }speedup = {1 \over (1 - p) + {p \over s}} $$

$$ Maximum\text{ }speedup = { 1 \over { {F}_ {par} \over n } + (1 - {F}_ {par} ) } $$

#
### Hiding latency with multiple threads
Reasons why a thread might be blocked...
* Waiting on I/O
* Waiting to acquire a mutex
* Waiting for another thread to complete an operation and notify a condition variable / populate a future
* Sleeping

> _", ...having blocked threads means you’re wasting CPU time."_ – pg. 279

Potential solutions:
* Use asynchronous I/O
* Get waiting thread to perform operation (like in our lock-free queue)
* If no thread has started the task, [start and perform the task on its own (extreme case)](https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/main/Chapter%2008%20-%20Designing%20concurrent%20code/lf_stack_sorter.cpp)

> _"Rather than adding threads to ensure that all available processors are being used, sometimes it pays to add threads to ensure that external events are handled in a timely manner to increase the responsiveness of the system."_ – pg. 280

#
### Improving responsiveness
> _"Most modern graphical user interface frameworks are event-driven;"_ – pg. 280

Previous issues with things like GUI's:
* Periodically suspend
* Insert `get_event()` / `process()` functions into just-launched function

[separate_gui.cpp](separate_gui.cpp)

For real, though - listing 8.6 is a hot mess; far too many sketches and not enough actual functioning code...if you're not going to write it in the book, at least include it in the online code samples.

#
### `par::for_each`
> _"...with a parallel implementation there’s no guarantee as to the order in which the elements will be processed, ..."_ – pg. 282

Pretty similar implementation to our `std::async` accumulate - albeit the author has decided to use a lambda function this time round like we did, passing by value instead of by ref, however, so I'll assume that's the safer way to do it.

[par_for_each.cpp](par_for_each.cpp)

We can make it more succinct like in listing 8.5 as well.

[parrer_for_each.cpp](parrer_for_each.cpp)

#
### `par::find`
> _"For algorithms such as std::find, the ability to complete “early” is an important property and not something to squander."_ – pg. 285

`std::promise` or `std::packaged_task`?
> _"If you want to stop on the first exception (even if you haven’t processed all elements), you can use `std::promise` to set both the value and the exception."</br>"On the other hand, if you want to allow the other workers to keep searching, you can use `std::packaged_task`, store all the exceptions, and then rethrow one of them if a match isn’t found."_ – pg. 285

After reading this, it makes more sense to opt for `std::promise` for this implementation.

[par_find.cpp](par_find.cpp)

Added a small benchmark to compare performance against `std::find` and it's mixed results - I may try again with Google Benchmark to see if there's any difference.

> _"...if a thread calling find_element either finds a match or throws an exception, all other threads will see done_flag set and will stop."_ – pg. 287

> _"If multiple threads find a match or throw at the same time, they’ll race to set the result in the promise. But this is a benign race condition; ..."_ – pg. 287

[par_async_find.cpp](par_async_find.cpp)

This is much cleaner than explicitly setting threads, and makes use of a helper function to separate the implementation and `std::atomic<bool>` from the main expected `par::find(c.begin(), c.end(), t)`-type structure.

There must be a better way to implement it though without `try / catch` blocks.

> _"A key feature...is that there’s no longer the guarantee that items are processed in the sequence that you get from `std::find`...<ins>**you can't process elements concurrently if the order matters**</ins>"_ – pg. 289

#
### `std::partial_sum`
Not an algorithm I've used very much (if at all), but I've attached as example below - it looks like something that would work well in a dynamic programming question.

[std_partial_sum.cpp](std_partial_sum.cpp)

To parallelise, we can split into chunks and add the tail elements of the previous chunk to the current chunk, e.g.
```cpp
{ 1, 2, 3, 4, 5, 6, 7, 8, 9 }
```
...becomes...
```cpp
{ 1, 2, 3 }, { 4, 5,  6 }, { 7,  8,  9 }
{ 1, 3, 6 }, { 4, 9, 15 }, { 7, 15, 24 }

// 6 is the last of chunk 1 - add 6 to chunk 2
{ 1, 3, 6 }, { 10, 15, 21 }, { 7, 15, 24 }

// 21 is the last of chunk 2 - add 21 to chunk 3
{ 1, 3, 6 }, { 10, 15, 21 }, { 28, 36, 45 } // book says 55?...
```
Issue raised [here](https://github.com/anthonywilliams/ccia_code_samples/issues/43) to highlight incorrect calculation in book.

...and a simple dynamic programming approach might be the following if we were to ignore iterators.
```cpp
void partial_sum(std::vector<int> &ivec) {
    if (ivec.empty())    { return; }
    if (ivec.size() < 2) { return ivec[0]; }
 
    for (int i = 1; i != ivec.size(); ++i) {
        ivec[i] += ivec[i - 1];
    }
}
```

It'll be interesting to see how these other algorithms compare to something even as simple as above with $O(n)$ runtime - the author claims it runs as fast as $O(log(n))$ if there are as many cores as elements.

[par_partial_sum.cpp](par_partial_sum.cpp)

Another spectacle of _"not production ready == doesn't compile"_ with this listing, and when it does run, it crashse, so the version I've uploaded above works, and I've raised a PR [here](https://github.com/anthonywilliams/ccia_code_samples/pull/44) explaining what changes were needed to get the code to compile and run as intended.

A few mouthful sentences here that could have been shortened, but they seem noteworthy.
> _"The main loop is the same as before, except this time you want the iterator that **points** to the last element in each block, rather than being the usual one past the end, so that you can do the forward propagation of the last element in each range"_ – pg. 293

> _"After you’ve spawned the thread, you can update the block start, remembering to advance it past that last element, and store the future for the last value in the current chunk into the vector of futures so it will be picked up next time around the loop."_ – pg. 293

Not sure on this one, though - `std::partial_sum` returns an `_OutputIterator`, in so far as Apple Clang goes.
> _"std::partial_sum doesn’t return a value"_ – pg. 293

> _"If you are not the first chunk, then there was a previous_end_value from the previ- ous chunk, so you need to wait for that"_ – pg. 293

> _"In order to maximize the parallelism of the algorithm, you then update the last element first"_ – pg. 293

This seems quite important in regards to throwing and propogating exceptions.
> _"Finally, if any of the operations threw an exception, you catch it and store it in the promise so it will propagate to the next chunk when it tries to get the previous end valuee."</br>"This will propagate all exceptions into the final chunk, which then rethrows, because you know you’re running on the main thread."_ – pg. 293

#
### Incremental pair-wise algorithm for partial sums
> _"The idea is to keep the threads in lock- step"_ – pg. 294

The author provides an implementation of a barrier, but we can also use `std::barrier` from C++20 (I'll try both once we get into the implementation).

[barrier.cpp](https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/dab2b33444d5db859bf52df27c6ba5ee8a526031/Chapter%2008%20-%20Designing%20concurrent%20code/barrier.cpp)

> _"When it reaches zero, the number of spaces is reset back to count, and the generation is increased to signal to the other threads that they can continue f. If the number of free spaces does not reach zero, you have to wait."_ – pg. 295

The author chooses to handle threads dropping out because it avoids threads doing unnecessary work (that's why we use atomic variables, to enable external syncrhonisation from multiple threads) - the use of atomic variables means we need to tweak the implementation slightly (not mentioned in the book).

[slightly_updated_barrier.cpp](https://github.com/ITHelpDec/CPP-Concurrency-in-Action/blob/833f264510a552b9bf811fedb9084d6c94ce3cd3/Chapter%2008%20-%20Designing%20concurrent%20code/barrier.cpp)

<details>
<summary><b>What's changed</b> - <i>(click to expand / collapse)</i></summary>

</br>

```diff
diff --git a/Chapter 08 - Designing concurrent code/barrier.cpp b/Chapter 08 - Designing concurrent code/barrier.cpp
index 903cf43..c2509f5 100644
--- a/Chapter 08 - Designing concurrent code/barrier.cpp	
+++ b/Chapter 08 - Designing concurrent code/barrier.cpp	
@@ -4,22 +4,32 @@
 
 class barrier {
 public:
-    explicit barrier(std::size_t count) : count_(count), spaces_(count_), generation_(0) { }
+    barrier(std::size_t count) : count_(count), spaces_(count), generation_(0) { }
     
     void wait()
     {
-        std::size_t my_gen = generation_;
+        std::size_t my_gen = generation_.load();
         
         if (!--spaces_) {
-            spaces_ = count_;
+            spaces_ = count_.load();
             ++generation_;
         } else {
-            while (generation_ == my_gen) { std::this_thread::yield(); }
+            while (generation_.load() == my_gen) { std::this_thread::yield(); }
+        }
+    }
+    
+    void done_waiting()
+    {
+        --count_;
+        
+        if (!--spaces_) {
+            spaces_ = count_.load();
+            ++generation_;
         }
     }
     
 private:
-    std::size_t count_;
+    std::atomic<std::size_t> count_;
     
     std::atomic<std::size_t> spaces_;
     std::atomic<std::size_t> generation_;
```

</details>

### Partially-complete sum
Another instance of incomplete code - my thanks go out to user [xxrlzzz](https://github.com/xxrlzzz) for the fix, as this one would have taken a while to debug (but again shows that the code wasn't tested).

[parrer_partial_sum.cpp](parrer_partial_sum.cpp)

I've attached a link to a PR [here]([https://github.com/anthonywilliams/ccia_code_samples/issues/5](https://github.com/anthonywilliams/ccia_code_samples/pull/45)) that addresses the issues in the code.

### Summary
This was a lengthy chapter - unfortunately, it was littered with what feels like more errors than previous chapters.

There were some good topics covered e.g. false sharing, propogating exceptions with `std::promise` / `std::packaged_rask`, opting for `std::async` over `std::thread` to prevent leaky threads, and templates to parallel approaches for popular algorithms.

It would have been nice to see more context around the likes of padding - the examples given felt a bit like "just chuck some padding in and see if it's better" instead of an educated way of making that decision.

Let's see what the next chapter on thread pools is like.

#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
