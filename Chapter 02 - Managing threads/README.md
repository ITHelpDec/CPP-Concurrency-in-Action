# C++ Concurrency in Action (2nd Edition)

## Highlights from Chapter 02 - "Managing threads"

### Most-vexing parse
> _"If you pass a temporary rather than a named variable, the syntax can be the same as that of a function declaration, in which case the compiler interprets it as such, rather than an object definition"_ – pg. 18, e.g. ...

```cpp
// declares a my_thread function that takes a single parameter...
// ...(of type pointer-to-a-function-taking-no-parameters-and-returning-a-background_task-object)...
// ...and returns a std::thread object, rather than launching a new thread
❌: std::thread my_thread(background_task());
```
```
Parentheses were disambiguated as a function declaration
Member reference base type 'std::thread (background_task (*)())' is not a structure or union
```
This can be _circumnavigated_ by using either extra surrounding parentheses or by using the new uniform initialisation syntax aka curly braces.
```cpp
✅: std::thread my_thread( (background_task()) );
✅: std::thread my_thread{background_task()};

// this also works, but wasn't listed in the examples
✅: std::thread my_thread(background_task{});
```
We can avoid the most-vexing parse entirely by using lambda's.
```cpp
✅: std::thread my_thread( [] () { std::cout << "woof\n"; } );
```
#
### ...work in progress
