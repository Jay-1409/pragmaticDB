#pragma once 
class DiskManager{
    public:
    void test();
    void WritePage(int page_id, const char* data);
    void ReadPage(int page_id, char* data);
};