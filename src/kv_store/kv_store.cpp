#include "kv_store/kv_store.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <cstdint>

static void write_str(std::ofstream& out, const std::string& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    out.write(str.data(), len);
}

static std::string read_str(std::ifstream& in) {
    uint32_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string s(len, '\0');
    if (len > 0) {
        in.read(&s[0], len);
    }
    return s;
}

KVStore:: KVStore(const std::string& path) : path_(path) {}

void KVStore::Put(const std::string& key, const std::string& value) {
    store_[key] = value;
}

std::string KVStore::Get(const std::string& key, bool& found) const {
    auto it = store_.find(key);
    if (it == store_.end()) {
        found = false;
        return "";
    }
    found = true;
    return it->second;
}

bool KVStore::Delete(const std::string& key) { return store_.erase(key) > 0;}

bool KVStore::Exists(const std::string& key) const { return store_.count(key) > 0;}

void KVStore::Flush() {
    std::filesystem::create_directories(std::filesystem::path(path_).parent_path());
    
    std::ofstream out(path_, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("KVStore: cannot open '" + path_ + "' for writing");
    }
    uint32_t num_entries = static_cast<uint32_t>(store_.size());
    out.write(reinterpret_cast<const char*>(&num_entries), sizeof(num_entries));

    for (const auto& [key, value] : store_) {
        write_str(out, key);
        write_str(out, value);
    }
}

void KVStore::Load() {
    if (!std::filesystem::exists(path_)) {
        return; // No file to load, start with empty store
    }

    std::ifstream in(path_, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("KVStore: cannot open '" + path_ + "' for reading");
    }

    store_.clear();
    uint32_t num_entries = 0;
    in.read(reinterpret_cast<char*>(&num_entries), sizeof(num_entries));
    
    for (uint32_t i = 0; i < num_entries; ++i) {
        std::string key = read_str(in);
        std::string value = read_str(in);
        store_[key] = value;
    }
}
