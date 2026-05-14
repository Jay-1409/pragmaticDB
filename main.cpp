#include "manager/disk_manager.h"
#include "manager/buffer_pool_manager.h"
#include "manager/page_data_manager.h"
#include<iostream>
#include <cstdio>
#include <cstring>
#include <string>
void test_responsiveness(DiskManager& dm, 
                        BufferPoolManager& bpm,
                        PageDataManager& pdm
                    ) {
    dm.test();
    bpm.test();
    pdm.test();
}
int main() {
    DiskManager dm;
    BufferPoolManager bpm(10, &dm);
    PageDataManager pdm;
    test_responsiveness(dm, bpm, pdm);
    return 0;
}
