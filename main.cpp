#include "disk_manager.h"
#include<iostream>
int main() {
    DiskManager dm;
    dm.test();
    dm.WritePage(0, "Hello, World!");
    char buffer[4096];
    dm.ReadPage(0, buffer);
    printf("Read from disk: %s\n", buffer);
    return 0;
}
