#include <iostream>
#include "tests.h"
#include "manager/table_manager.h"
#include "utils/table_iterator.h"
#include "catalog/schema.h"

#define ASSERT(expr) \
    if (!(expr)) { \
        std::cout << "[FAIL] " << #expr << " on line " << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        std::cout << "[PASS] " << #expr << "\n"; \
        passed_count++; \
    }

void test::TestTableIteratorClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing TableIterator Class ---\n";

    Schema schema({Column("id", TypeId::INTEGER), Column("is_active", TypeId::BOOLEAN)});
    TableManager table;

    // ── Test 1: Empty table ────────────────────────────────────────────────
    std::cout << "\n[Test 1] Empty table: Begin == End\n";
    ASSERT(table.Begin(schema) == table.End(schema));

    // ── Test 2: Insert 3 tuples, iterate all ─────────────────────────────
    std::cout << "\n[Test 2] Insert 3 tuples, scan all\n";
    Value v1(TypeId::INTEGER); v1.Set<int32_t>(1);
    Value b1(TypeId::BOOLEAN); b1.Set<int8_t>(true);

    Value v2(TypeId::INTEGER); v2.Set<int32_t>(2);
    Value b2(TypeId::BOOLEAN); b2.Set<int8_t>(false);

    Value v3(TypeId::INTEGER); v3.Set<int32_t>(3);
    Value b3(TypeId::BOOLEAN); b3.Set<int8_t>(true);

    RecordId rid1 = table.InsertTuple(Tuple({v1, b1}, schema));
    RecordId rid2 = table.InsertTuple(Tuple({v2, b2}, schema));
    RecordId rid3 = table.InsertTuple(Tuple({v3, b3}, schema));

    ASSERT(rid1.page_id != INVALID_PAGE_ID);
    ASSERT(rid2.page_id != INVALID_PAGE_ID);
    ASSERT(rid3.page_id != INVALID_PAGE_ID);

    int count = 0;
    for (auto it = table.Begin(schema); it != table.End(schema); ++it) {
        count++;
    }
    ASSERT(count == 3);

    // ── Test 3: Values are correct after scan ─────────────────────────────
    std::cout << "\n[Test 3] Values are correct on scan\n";
    auto it = table.Begin(schema);
    ASSERT((*it).GetValue(schema, 0).Get<int32_t>() == 1);
    ASSERT((*it).GetValue(schema, 1).Get<int8_t>()  == true);
    ++it;
    ASSERT((*it).GetValue(schema, 0).Get<int32_t>() == 2);
    ASSERT((*it).GetValue(schema, 1).Get<int8_t>()  == false);
    ++it;
    ASSERT((*it).GetValue(schema, 0).Get<int32_t>() == 3);
    ++it;
    ASSERT(it == table.End(schema));

    // ── Test 4: Delete middle tuple, scan skips it ────────────────────────
    std::cout << "\n[Test 4] Delete middle tuple, iterator skips it\n";
    table.DeleteTuple(rid2);

    int count_after_delete = 0;
    for (auto it2 = table.Begin(schema); it2 != table.End(schema); ++it2) {
        count_after_delete++;
    }
    ASSERT(count_after_delete == 2);

    std::cout << "\nTableIterator Class test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
