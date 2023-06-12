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

> _"As with most of the examples, this is intended to demonstrate an idea rather than being production-ready code.""

...from pg. 255 really isn't good enough - no book should be published with this many mistakes and bad practices under the guise of it not being "production-ready".

It's instructional, and as such should not only compile and run, but run as intended.

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
