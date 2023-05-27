// #include <functional>
// #include <vector>
// #include <memory>
// #include <unordered_map>
// #include <chrono>

#include <list>
#include <shared_mutex>
#include <iostream>
#include <random>
#include <thread>

namespace ts {
template <typename K, typename V, typename H = std::hash<K>>
class map {
public:
    typedef K key_type;
    typedef V value_type;
    typedef H hash_type;
    
    map(std::size_t num_buckets = 19, const H &hasher = H())
        : buckets_(num_buckets), hasher_(hasher) {
        for (std::size_t i = 0; i != num_buckets; ++i) {
            // buckets_[i].reset(new bucket_type);
            buckets_[i] = std::make_unique<bucket_type>();
            
        }
    }
    
    map(const map&) = delete;
    map& operator=(const map&) = delete;
    
    V value_for(const K &key, const V &default_value = V()) const
    {
        return get_bucket(key).value_for(key, default_value);
    }
    
    void add_or_update_mapping(const K &key, const V &value)
    {
        return get_bucket(key).add_or_update_mapping(key, value);
    }
    
    void remove_mapping(const K &key)
    {
        get_bucket(key).remove_mapping(key);
    }
    
private:
    class bucket_type {
    public:
        
        // 'this' argument to member function 'find_entry_for' has type...
        // 'const ts::map<int, int>::bucket_type', ...
        // ...but function is not marked const
        //
        // V value_for(const K &key, const V &default_value) const
        
        V value_for(const K &key, const V &default_value)
        {
            std::shared_lock<std::shared_mutex> lock(sm_);
            const_bucket_iterator found_entry = find_entry_for(key);
            
            return found_entry == data_.end() ? default_value : found_entry->second;
        }
        
        void add_or_update_mapping(const K &key, const V &value)
        {
            std::shared_lock<std::shared_mutex> lock(sm_);
            bucket_iterator found_entry = find_entry_for(key);
            
            if (found_entry == data_.end()) {
                // data_.push_back(bucket_value(key, value));
                data_.emplace_back(key, value);
            } else {
                found_entry->second = value;
            }
        }
        
        void remove_mapping(const K &key)
        {
            std::unique_lock<std::shared_mutex> lock(sm_);
            bucket_iterator found_entry = find_entry_for(key);
            
            if (found_entry != data_.end()) { data_.erase(found_entry); }
        }
        
    private:
        typedef std::pair<K, V> bucket_value;
        typedef std::list<bucket_value> bucket_data;
        typedef typename bucket_data::iterator bucket_iterator;
        typedef typename bucket_data::const_iterator const_bucket_iterator;
        
        bucket_data data_;
        mutable std::shared_mutex sm_;
        
        // No viable conversion from returned value of type...
        // 'std::__list_const_iterator<std::pair<int, int>, void *>'
        // ...to function return type...
        // 'ts::map<int, int>::bucket_type::bucket_iterator'
        // (aka '__list_iterator<std::pair<int, int>, void *>')
        //
        // bucket_iterator find_entry_for(const K &key) const
       
        bucket_iterator find_entry_for(const K &key)
        {
            return std::find_if(data_.begin(), data_.end(), [&] (const bucket_value &bv) {
                return bv.first == key;
            });
        }
        
    };
    
    std::vector<std::unique_ptr<bucket_type>> buckets_;
    H hasher_;
    
    bucket_type& get_bucket(const K &key) const
    {
        const std::size_t bucket_index = hasher_(key) % buckets_.size();
        return *buckets_[bucket_index];
    }
    
};
} // namespace ts (threadsafe)

int num() {
    static std::default_random_engine e;
    static std::uniform_int_distribution u(2, 5);
    return u(e);
}

void populate_uimap(std::unordered_map<int, int> &uimap) {
    for (int i = 0; i != uimap.size(); ++i) {
        uimap.emplace(num(), num());
    }
}

void clear_uimap(std::unordered_map<int, int> &uimap) {
    for (auto it = uimap.begin(); it != uimap.end(); ++it) {
        uimap.erase(it);
    }
}

void populate_tsmap(ts::map<int, int> &tsmap) {
    for (int i = 0; i != 100; ++i) {
        tsmap.add_or_update_mapping(num(), num());
    }
}

void clear_tsmap(ts::map<int, int> &tsmap) {
    for (int i = 0; i != 100; ++i) {
        tsmap.remove_mapping(num());
    }
}

int main()
{
    const std::size_t sz = 1000;
    
    std::cout << "Initialising and timing uimap...";
    
    auto start1 = std::chrono::high_resolution_clock::now();
    std::unordered_map<int, int> uimap(sz);
    populate_uimap(uimap);
    clear_uimap(uimap);
    auto stop1 = std::chrono::high_resolution_clock::now();
    auto result1 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop1 - start1).count();
    
    std::cout << "...it took " << result1 << "ns to populate and clear the map.\n";
    
    std::cout << "Initialising and timing tsmap...";
    
    auto start2 = std::chrono::high_resolution_clock::now();
    ts::map<int, int> tsmap(sz);
    std::thread t1(populate_tsmap, std::ref(tsmap));
    std::thread t2(clear_tsmap, std::ref(tsmap));
    
    t1.join();
    t2.join();
    
    auto stop2 = std::chrono::high_resolution_clock::now();
    auto result2 = std::chrono::duration_cast<std::chrono::nanoseconds>(stop2 - start2).count();
    
    std::cout << "...it took " << result2 << "ns to populate and clear the map.\n";
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising and timing uimap......it took 8000ns to populate and clear the map.
// Initialising and timing tsmap......it took 238875ns to populate and clear the map.
// Program ended with exit code: 0
