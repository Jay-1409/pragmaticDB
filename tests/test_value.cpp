#include <iostream>
#include <cstring>
#include "tests.h"
#include "type/value.h"
#include "type/type_id.h"

// Basic assertion macro
#define ASSERT(expr) \
    if (!(expr)) { \
        std::cout << "[FAIL] " << #expr << " on line " << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        std::cout << "[PASS] " << #expr << "\n"; \
        passed_count++; \
    }

void test::TestValueClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing Value Class ---\n";

    // 1. Test Integer Serialization
    {
        Value int_val(TypeId::INTEGER);
        int_val.Set<int32_t>(42069);
        ASSERT(int_val.Get<int32_t>() == 42069);

        // Serialize
        char buffer[4];
        int_val.SerializeToChar(buffer);

        // Deserialize into a new value
        Value new_val(TypeId::INVALID); // Start with something invalid
        new_val.DeserializeFromChar(buffer, TypeId::INTEGER);

        ASSERT(new_val.Get<int32_t>() == 42069);
    }

    // 2. Test Boolean Serialization
    {
        Value bool_val(TypeId::BOOLEAN);
        bool_val.Set<int8_t>(true);
        ASSERT(bool_val.Get<int8_t>() == true);

        // Serialize
        char buffer[1];
        bool_val.SerializeToChar(buffer);

        // Deserialize into a new value
        Value new_val(TypeId::INVALID); 
        new_val.DeserializeFromChar(buffer, TypeId::BOOLEAN);

        ASSERT(new_val.Get<int8_t>() == true);
    }

    std::cout << "\nValue Class test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
