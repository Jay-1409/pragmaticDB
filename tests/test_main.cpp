#include <iostream>
#include "tests.h"

int main() {
    test tests;
    tests.TestBufferPoolManager();
    tests.TestRecordManager();
    tests.TestValueClass();
    tests.TestSchemaClass();
    tests.TestTupleClass();
    tests.TestTableManagerClass();
    tests.TestCatalogClass();
    tests.TestTableIteratorClass();
    return 0;
}
