# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 04 - "Synchronising concurrent operations"

#
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

### ...work in progress
