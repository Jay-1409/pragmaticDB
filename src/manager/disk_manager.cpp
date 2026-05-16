#include "manager/disk_manager.h"
#include <iostream>
#include <fstream>
const int PAGE_SIZE = 4096;

DiskManager::DiskManager(const std::string& filename) : filename_(filename) {}

void DiskManager::test() {
    std::cout << "Disk manager is responding" << std::endl;
}
void DiskManager::WritePage(int page_id, const char* data) {
    std::fstream file(filename_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        file.open(filename_, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename_, std::ios::in | std::ios::out | std::ios::binary);
    }
    file.seekp(PAGE_SIZE * page_id);
    file.write(data, PAGE_SIZE);
    file.close();   
}

void DiskManager::ReadPage(int page_id, char* data) {
    std::ifstream file(filename_, std::ios::in | std::ios::binary);
    if (!file) {
        std::cerr << "File not found: " << filename_ << std::endl;
        return;
    }
    file.seekg(PAGE_SIZE * page_id);
    file.read(data, PAGE_SIZE);
    file.close();
}