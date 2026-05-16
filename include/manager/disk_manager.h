#pragma once 
#include <string>

class DiskManager {
    public:
    explicit DiskManager(const std::string& filename = "data.db");
    void test();
    void WritePage(int page_id, const char* data);
    void ReadPage(int page_id, char* data);
    private:
    std::string filename_;
};