#include <iostream>
#include "tests.h"
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

void test::TestSchemaClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing Schema & Column Classes ---\n";

    std::vector<Column> columns = {
        Column("id", TypeId::INTEGER),
        Column("is_active", TypeId::BOOLEAN),
        Column("age", TypeId::INTEGER)
    };
    Schema schema(columns);

    ASSERT(schema.GetColumnCount() == 3);
    ASSERT(schema.GetLength() == 9); // 4 + 1 + 4
    
    ASSERT(schema.GetColOffset(0) == 0);
    ASSERT(schema.GetColOffset(1) == 4);
    ASSERT(schema.GetColOffset(2) == 5);

    ASSERT(schema.GetColumn(0).GetName() == "id");
    ASSERT(schema.GetColumn(1).GetType() == TypeId::BOOLEAN);
    ASSERT(schema.GetColumn(2).GetLength() == 4);

    std::cout << "\nSchema Class test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
