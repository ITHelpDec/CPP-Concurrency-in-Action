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
### Other golden nuggest
Plenty of golden nuggest so far in this chapter.
> _"You need to ensure that data can’t be accessed outside the protection of the mutex lock and that there are no race conditions inherent in the interface, ..."_ – pg. 176

### ...work in progress
#
### If you've found anything from this repo useful, please consider contributing towards the only thing that makes it all possible – my unhealthy relationship with 90+ SCA score coffee beans.

<a href="https://www.buymeacoffee.com/ITHelpDec"><img src="https://img.buymeacoffee.com/button-api/?text=Buy me a coffee&emoji=&slug=ITHelpDec&button_colour=FFDD00&font_colour=000000&font_family=Cookie&outline_colour=000000&coffee_colour=ffffff" /></a>
