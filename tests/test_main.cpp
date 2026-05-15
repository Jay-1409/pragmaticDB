#include <iostream>
#include "tests.h"

int main() {
    test tests;
    tests.TestBufferPoolManager();
    tests.TestRecordManager();
    tests.TestValueClass();
    tests.TestSchemaClass();
    tests.TestTupleClass();
    tests.TestTableHeapClass();
    return 0;
}
