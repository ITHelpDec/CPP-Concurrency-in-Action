#include <future>
#include <vector>
#include <iostream>
#include <numeric>

int process_chunk(std::vector<int>::const_iterator b, std::vector<int>::const_iterator e) {
    return std::accumulate(b, e, 0);
}

template <typename ForwardIt>
int process_chunk_template(ForwardIt b, ForwardIt e) {
    return std::accumulate(b, e, 0);
}

int gather_results(const std::vector<int> &ivec) {
    return std::accumulate(ivec.begin(), ivec.end(), 0);
}

std::future<int> process_data(const std::vector<int> &ivec) {
    const std::size_t chunk_size = ivec.size() / 4;

    std::size_t remaining_size, this_chunk_size;
    
    std::vector<std::future<int>> results;
    
    for (auto it = ivec.begin(); it != ivec.end(); /*...*/) {
        std::size_t remaining_size = std::distance(it, ivec.end());
        std::size_t this_chunk_size = std::min(remaining_size, chunk_size);
        
        // standard function
        results.push_back(std::async(process_chunk, it, it + this_chunk_size));
        
        // function templates
        
        // - explicit specialisation
        // results.push_back(std::async(process_chunk_template<std::vector<int>::const_iterator>, it, it + this_chunk_size));
        
        // - decltype shorthand
        // results.push_back(std::async(process_chunk_template<decltype(it)>, it, it + this_chunk_size));
        
        // - lambda shorthand
        // results.push_back(std::async([=] () {
        //     return process_chunk_template(it, it + this_chunk_size);
        // }));
        
        it += this_chunk_size;
    }
    
    return std::async([all_results = std::move(results)] () mutable {
        std::vector<int> v;
        v.reserve(all_results.size());
        
        for (auto &fut : all_results) { v.push_back(fut.get()); }
        
        return gather_results(v);
    });
}

int main()
{
    std::vector<int> ivec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    
    auto fut = process_data(ivec);
    
    std::cout << "fut.get(): " << fut.get() << '\n';
    
    return 0;
}
