#include <iostream>
#include "tests.h"
#include "query/parser.h"
#include "query/executor.h"
#include "catalog/catalog.h"

#define ASSERT(expr) \
    if (!(expr)) { \
        std::cout << "[FAIL] " << #expr << " on line " << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        std::cout << "[PASS] " << #expr << "\n"; \
        passed_count++; \
    }

void test::TestQueryEngineClass() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing Query Engine ---\n";

    Catalog catalog;
    Executor executor(catalog);
    Parser parser;

    // 1. CREATE TABLE
    auto stmt1 = parser.Parse("CREATE TABLE users (id INTEGER, is_active BOOLEAN);");
    ASSERT(stmt1 != nullptr);
    ASSERT(stmt1->type == StatementType::CREATE_TABLE);
    auto res1 = executor.Execute(*stmt1);
    ASSERT(res1.success);

    // 2. INSERT (row 1)
    auto stmt2 = parser.Parse("INSERT INTO users VALUES (42, true);");
    ASSERT(stmt2 != nullptr);
    ASSERT(stmt2->type == StatementType::INSERT);
    auto res2 = executor.Execute(*stmt2);
    ASSERT(res2.success);

    // 3. INSERT (row 2)
    auto stmt3 = parser.Parse("INSERT INTO users VALUES (99, false);");
    ASSERT(stmt3 != nullptr);
    auto res3 = executor.Execute(*stmt3);
    ASSERT(res3.success);

    // 4. SELECT
    auto stmt4 = parser.Parse("SELECT * FROM users;");
    ASSERT(stmt4 != nullptr);
    ASSERT(stmt4->type == StatementType::SELECT);
    auto res4 = executor.Execute(*stmt4);
    ASSERT(res4.success);
    ASSERT(res4.rows.size() == 2);
    
    // Check first row
    ASSERT(res4.rows[0].size() == 2);
    ASSERT(res4.rows[0][0] == "42");
    ASSERT(res4.rows[0][1] == "true");

    // Check second row
    ASSERT(res4.rows[1].size() == 2);
    ASSERT(res4.rows[1][0] == "99");
    ASSERT(res4.rows[1][1] == "false");

    std::cout << "\nQuery Engine test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
