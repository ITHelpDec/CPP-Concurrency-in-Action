#include <map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <iostream>

struct dns_entry {
    std::string ip_addr = "0.0.0.0";
};

class dns_cache {
public:
    dns_entry find_entry(const std::string &domain) const
    {
        std::shared_lock<std::shared_mutex> lock(entry_mutex);
        
        // const std::map<std::string, dns_entry>::const_iterator it = entries.find(domain);
        // return (it==entries.end())?dns_entry():it->second;
        
        // come on, bro...it's 2023...
        // no harm in using a bit of whitespace and `auto` for iterators
        auto it = entries.find(domain);
        return it == entries.end() ? dns_entry() // default-initialised
                                   : it->second; // not the most declarative, but it'll do
    }
    
    void update_or_add_entry(const std::string &domain, const dns_entry &dns_details)
    {
        std::lock_guard<std::shared_mutex> lock(entry_mutex);
        entries[domain] = dns_details;
    }
    
private:
    std::map<std::string, dns_entry> entries;
    mutable std::shared_mutex entry_mutex;
};

int main()
{
    dns_cache cache;
    
    cache.update_or_add_entry("Google", dns_entry{"8.8.8.8"} );
    cache.update_or_add_entry("Google", dns_entry{"8.8.4.4"} );
    
    std::cout << "Searching for results...\n";
    
    auto result = cache.find_entry("Google");
    std::cout << "\"Google\": " << result.ip_addr << '\n';
    
    result = cache.find_entry("Amazon");
    std::cout << "\"Amazon\": " << result.ip_addr << '\n';
    
    return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// OUTPUT  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// Searching for results...
// "Google": 8.8.4.4
// "Amazon": 0.0.0.0
// Program ended with exit code: 0
