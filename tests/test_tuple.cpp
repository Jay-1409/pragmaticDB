#include <iostream>
#include "tests.h"
#include "type/tuple.h"
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

void test::TestTupleClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing Tuple Class ---\n";

    Schema schema({Column("id", TypeId::INTEGER), Column("is_active", TypeId::BOOLEAN)});
    
    Value val1(TypeId::INTEGER); 
    val1.Set<int32_t>(42069);
    
    Value val2(TypeId::BOOLEAN); 
    val2.Set<int8_t>(true);
    
    std::vector<Value> vals = {val1, val2};
    Tuple tuple(vals, schema);
    
    ASSERT(tuple.GetLength() == 5);
    ASSERT(tuple.GetValue(schema, 0).Get<int32_t>() == 42069);
    ASSERT(tuple.GetValue(schema, 1).Get<int8_t>() == true);

    std::cout << "\nTuple Class test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
