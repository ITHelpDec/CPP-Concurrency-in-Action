#include <thread>
#include <atomic>
#include <string>
#include <random>
#include <iostream>

// not included by author...
int num() {
    static std::default_random_engine e;
    static std::uniform_int_distribution u(1, 10000000);
    return u(e);
}

std::thread task_thread;
std::atomic<bool> task_cancelled(false);

// not included by author...
std::atomic<bool> completed(false);

// not included by author...
std::atomic<int> operations(0);

// not included by author...
struct event_data {
    int type;
};

// not included by author...
enum { quit, start_task, stop_task, task_complete } msg;

// not included by author...
int get_event() {
    int n = num();
    return n == 69 ? 69 : n;
}

// not included by author...
bool task_completed() {
    completed.store(true);
    return completed;
}

// not included by author...
void do_next_operation() { ++operations; }

// not included by author...
void perform_cleanup() { }

// not included by author...
void post_gui_thread(int x) { }

void task() {
    while (!task_completed() && !task_cancelled)
        do_next_operation();
    
    if (task_cancelled) {
        perform_cleanup();
    } else {
        post_gui_thread(task_complete);
    }
}

// not included by author...
void display_results() {
    std::cout << "yeah, boi...\n";
}

void process(const event_data &event) {
    switch (event.type) {
        case start_task:
            task_cancelled = false;
            task_thread = std::thread(task);
            task_thread.detach();
            break;
        case stop_task:
            std::cout << "stopping task...\n";
            task_cancelled = true;
            task_thread.join();
            break;
        case task_complete:
            task_thread.join();
            display_results();
            break;
        default:
            break;
    }
}

void gui_thread() {
    while (true) {
        // event_data event = get_event();
        event_data event = { get_event() };
        if (event.type == 69) { break; }
        
        process(event);
    }
}

int main()
{
    std::cout << "running gui thread...";
    gui_thread();
    
    std::cout << "nice 🤌🏻\n";
    
    std::cout << operations << " operations performed.\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// running gui thread...nice 🤌🏻
// 0 operations performed.
// Program ended with exit code: 0
