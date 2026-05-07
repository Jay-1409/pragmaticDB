#include "../include/disk_manager.h"
#include <iostream>
#include <fstream>
const int PAGE_SIZE = 4096;
void DiskManager::test() {
    std::cout << "Disk manager is responding" << std::endl;
}
void DiskManager::WritePage(int page_id, const char* data) {
    /** Each page had a size of PAGE_SIZE
     * offset = PAGE_SIZE * page_id
     */
    std::fstream file("data.db", std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open("data.db", std::ios::out | std::ios::binary);
        file.close();
        file.open("data.db", std::ios::in | std::ios::out | std::ios::binary);
    }
    file.seekp(PAGE_SIZE * page_id);
    file.write(data, PAGE_SIZE);
    file.close();   
}

void DiskManager::ReadPage(int page_id, char* data) {
    std::ifstream file("data.db", std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "File not found!" << std::endl;
        return;
    }
    file.seekg(PAGE_SIZE * page_id);
    file.read(data, PAGE_SIZE);
    file.close();
}