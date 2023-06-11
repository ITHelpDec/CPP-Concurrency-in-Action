// #include <exception>
#include <stack>
// #include <mutex>
// #include <memory>
// #include <thread>
// #include <vector>
#include <future>
#include <list>
#include <iostream>

// struct empty_stack: std::exception
// {
//     const char* what() const throw()
//     {
//         return "empty stack";
//     }
// };

template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data=other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));
    }
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        // if(data.empty()) throw empty_stack();
        if(data.empty()) return std::shared_ptr<T>();
        std::shared_ptr<T> const res(
            std::make_shared<T>(std::move(data.top())));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        // if(data.empty()) throw empty_stack();
        if(data.empty()) return;
        value=std::move(data.top());
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<typename T>
struct sorter
{
    struct chunk_to_sort
    {
        std::list<T> data;
        std::promise<std::list<T> > promise;
    };

    // thread_safe_stack<chunk_to_sort> chunks;
    threadsafe_stack<chunk_to_sort> chunks;
    std::vector<std::thread> threads;
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;

    sorter():
        max_thread_count(std::thread::hardware_concurrency()-1),
        end_of_data(false)
    {}

    ~sorter()
    {
        end_of_data=true;
        for(unsigned i=0;i<threads.size();++i)
        {
            threads[i].join();
        }
    }

    void try_sort_chunk()
    {
        // boost::shared_ptr<chunk_to_sort > chunk=chunks.pop();
        std::shared_ptr<chunk_to_sort > chunk=chunks.pop();
        if(chunk)
        {
            sort_chunk(chunk);
        }
    }

    std::list<T> do_sort(std::list<T>& chunk_data)
    {
        if(chunk_data.empty())
        {
            return chunk_data;
        }

        std::list<T> result;
        result.splice(result.begin(),chunk_data,chunk_data.begin());
        T const& partition_val=*result.begin();

        typename std::list<T>::iterator divide_point=
            std::partition(chunk_data.begin(),chunk_data.end(),
                           [&](T const& val){return val<partition_val;});
        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(),
                                    chunk_data,chunk_data.begin(),
                                    divide_point);

        std::future<std::list<T> > new_lower=
            new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk));
        if(threads.size()<max_thread_count)
        {
            threads.push_back(std::thread(&sorter<T>::sort_thread,this));
        }

        std::list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(),new_higher);
        while(new_lower.wait_for(std::chrono::seconds(0)) !=
              std::future_status::ready)
        {
            try_sort_chunk();
        }

        result.splice(result.begin(),new_lower.get());
        return result;
    }

    // void sort_chunk(boost::shared_ptr<chunk_to_sort > const& chunk)
    void sort_chunk(std::shared_ptr<chunk_to_sort > const& chunk)
    {
        chunk->promise.set_value(do_sort(chunk->data));
    }

    void sort_thread()
    {
        while(!end_of_data)
        {
            try_sort_chunk();
            std::this_thread::yield();
        }
    }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if(input.empty())
    {
        return input;
    }
    sorter<T> s;
    return s.do_sort(input);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

template <typename T>
void print_list(const std::list<T> &il) {
    for (auto &&e : il) {
        std::cout << e << ' ';
    } std::cout << '\n';
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int main()
{
    std::list<int> il = { 1, 4, 5, 3, 6, 7, 4 };
    std::cout << "before: ";
    print_list(il);

    auto sorted_il = parallel_quick_sort(il);
    std::cout << "after:  ";
    print_list(sorted_il);

    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// before: 1 4 5 3 6 7 4
// after:  1 3 4 4 5 6 7
// Program ended with exit code: 0
