#include <iostream>
#include "tests.h"
#include "catalog/catalog.h"
#include "type/tuple.h"

// Basic assertion macro
#define ASSERT(expr) \
    if (!(expr)) { \
        std::cout << "[FAIL] " << #expr << " on line " << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        std::cout << "[PASS] " << #expr << "\n"; \
        passed_count++; \
    }

void test::TestCatalogClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing Catalog Class ---\n";

    Catalog catalog;
    Schema schema({Column("id", TypeId::INTEGER), Column("is_active", TypeId::BOOLEAN)});
    
    TableInfo* table_info = catalog.CreateTable("users", schema);
    ASSERT(table_info != nullptr);
    ASSERT(table_info->name_ == "users");
    ASSERT(table_info->oid_ == 0);
    
    TableInfo* fetched_by_name = catalog.GetTable("users");
    ASSERT(fetched_by_name != nullptr);
    ASSERT(fetched_by_name->oid_ == 0);
    
    TableInfo* fetched_by_oid = catalog.GetTable(0);
    ASSERT(fetched_by_oid != nullptr);
    ASSERT(fetched_by_oid->name_ == "users");
    
    bool caught_exception = false;
    try {
        catalog.GetTable("non_existent");
    } catch (const std::runtime_error& e) {
        caught_exception = true;
    }
    ASSERT(caught_exception == true);
    
    caught_exception = false;
    try {
        catalog.CreateTable("users", schema);
    } catch (const std::runtime_error& e) {
        caught_exception = true;
    }
    ASSERT(caught_exception == true);

    std::cout << "\nCatalog Class test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
