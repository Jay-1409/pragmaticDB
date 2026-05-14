#include <iostream>
#include "tests.h"

int main() {
    test tests;
    tests.TestBufferPoolManager();
    tests.TestRecordManager();
    return 0;
}
