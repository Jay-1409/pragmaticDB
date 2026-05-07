#include "disk_manager.h"
#include "buffer_pool_manager.h"
#include<iostream>
void test_all(DiskManager& dm, BufferPoolManager& bpm) {
    dm.test();
    bpm.test();
}
int main() {
    DiskManager dm;
    BufferPoolManager bpm(10, &dm);
    test_all(dm, bpm);
    return 0;
}
