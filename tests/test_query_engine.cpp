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

    // 5. DELETE WHERE — remove only row with id = 42
    auto stmt5 = parser.Parse("DELETE FROM users WHERE id = 42;");
    ASSERT(stmt5 != nullptr);
    ASSERT(stmt5->type == StatementType::DELETE);
    auto res5 = executor.Execute(*stmt5);
    ASSERT(res5.success);
    ASSERT(res5.message == "1 row(s) deleted.");

    // 6. SELECT — should now have only id=99 left
    auto stmt6 = parser.Parse("SELECT * FROM users;");
    auto res6 = executor.Execute(*stmt6);
    ASSERT(res6.success);
    ASSERT(res6.rows.size() == 1);
    ASSERT(res6.rows[0][0] == "99");

    // 7. DELETE WHERE non-existent value — should delete 0 rows
    auto stmt7 = parser.Parse("DELETE FROM users WHERE id = 999;");
    auto res7 = executor.Execute(*stmt7);
    ASSERT(res7.success);
    ASSERT(res7.message == "0 row(s) deleted.");

    // 8. DELETE WHERE wrong column — should return an error
    auto stmt8 = parser.Parse("DELETE FROM users WHERE nonexistent = 1;");
    auto res8 = executor.Execute(*stmt8);
    ASSERT(!res8.success);

    // 9. DELETE all rows (no WHERE)
    auto stmt9 = parser.Parse("DELETE FROM users;");
    ASSERT(stmt9 != nullptr);
    auto res9 = executor.Execute(*stmt9);
    ASSERT(res9.success);
    ASSERT(res9.message == "1 row(s) deleted."); // only 1 row left after step 5

    // 10. SELECT — table should now be empty
    auto stmt10 = parser.Parse("SELECT * FROM users;");
    auto res10 = executor.Execute(*stmt10);
    ASSERT(res10.success);
    ASSERT(res10.rows.size() == 0);

    std::cout << "\nQuery Engine test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
