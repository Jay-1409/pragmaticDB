#pragma once

#include <string>
#include <unordered_map>

class KVStore {
    public:
        explicit KVStore(const std:: string& path = "data/kv_store.db");

        // Insert or update a key-value pair
        void Put(const std:: string& key, const std::string& value);

        // Get a value by key
        std::string Get(const std:: string& key, bool& found) const;

        // Delete a key-value pair
        bool Delete(const std:: string& key);

        // Existence check for a key
        bool Exists(const std:: string& key) const;

        // Persis the current state to disk
        void Flush();

        // Load persisted state from disk
        void Load();

        // Number of entries in the store
        size_t Size() const { return store_.size();}
    
    private:
        std::string path_;
        std::unordered_map<std::string, std::string> store_;
};
