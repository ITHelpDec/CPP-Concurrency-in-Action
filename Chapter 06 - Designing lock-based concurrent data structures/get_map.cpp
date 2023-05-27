// #include <functional>
// #include <vector>
// #include <memory>
// #include <unordered_map>
// #include <chrono>

#include <list>
#include <shared_mutex>
#include <iostream>
#include <map>
#include <random>

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
    
    std::map<K, V> get_map() const
    {
        std::vector<std::unique_lock<std::shared_mutex>> locks;
        
        for (const auto &b : buckets_) {
            locks.emplace_back(std::unique_lock<std::shared_mutex>(b->sm_));
        }
        
        std::map<K, V> result;
        
        for (const auto &b : buckets_) {
            // Unknown type name 'bucket_iterator'
            // - bucket_iterator does not exist in scope of map, but of bucket_type
            // for (bucket_iterator it = b.data_.begin(); it != b.data_.end(); ++it) {
            for (auto it = b->data_.begin(); it != b->data_.end(); ++it) {
                result.insert(*it);
            }
        }
        
        return result;
    }
    
private:
    class bucket_type {
        // not mentioned in the book, but this is required...
        friend std::map<K, V> ts::map<K, V>::get_map() const;
    public:
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

int main()
{
    std::cout << "Initialising and populating ts::map<int, int>...\n";
    ts::map<int, int> tsmap;
    for (int i = 0; i != 10; ++i) { tsmap.add_or_update_mapping(num(), num()); }
    
    std::cout << "Copying our ts::map across to a std::map<int, int>...\n";
    std::map map = tsmap.get_map(); // C++17 CTAD (class template argument deduction)
    
    std::cout << "map: ";
    for (const auto &[k, v] : map) {
        std::cout << "{ " << k << ", " << v << " }" << ' ';
    } std::cout << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  OUTPUT - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Initialising and populating ts::map<int, int>...
// Copying our ts::map across to a std::map<int, int>...
// map: { 2, 3 } { 3, 2 } { 4, 4 }
// Program ended with exit code: 0
