#include <iostream>
#include "tests.h"
#include "manager/table_manager.h"
#include "catalog/schema.h"

// Basic assertion macro
#define ASSERT(expr) \
    if (!(expr)) { \
        std::cout << "[FAIL] " << #expr << " on line " << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        std::cout << "[PASS] " << #expr << "\n"; \
        passed_count++; \
    }

void test::TestTableManagerClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing TableManager Class ---\n";

    TableManager table_heap;
    Schema schema({Column("id", TypeId::INTEGER), Column("is_active", TypeId::BOOLEAN)});
    
    Value val1(TypeId::INTEGER); val1.Set<int32_t>(777);
    Value val2(TypeId::BOOLEAN); val2.Set<int8_t>(false);
    Tuple tuple({val1, val2}, schema);
    
    RecordId rid = table_heap.InsertTuple(tuple);
    ASSERT(rid.page_id != -1);
    
    Tuple fetched_tuple = table_heap.GetTuple(rid, schema);
    ASSERT(fetched_tuple.GetLength() == 5);
    ASSERT(fetched_tuple.GetValue(schema, 0).Get<int32_t>() == 777);
    ASSERT(fetched_tuple.GetValue(schema, 1).Get<int8_t>() == false);

    std::cout << "\nTableManager Class test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
