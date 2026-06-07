#include <iostream>
#include "tests.h"
#include "catalog/catalog.h"
#include "query/executor.h"
#include "query/parser.h"

#define ASSERT_JOIN(expr) \
    if (!(expr)) { \
        std::cout << "[FAIL] " << #expr << " on line " << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        passed_count++; \
    }

void test::TestJoin() {
    int passed_count = 0;
    int failed_count = 0;

    std::cout << "\n--- Testing JOIN ---\n";

    Catalog catalog;
    Executor executor(catalog);
    Parser parser;

    auto exec = [&](const std::string& sql) -> QueryResult {
        auto stmt = parser.Parse(sql);
        ASSERT_JOIN(stmt != nullptr);
        if (!stmt) return {false, "Parse failed", {}};
        return executor.Execute(*stmt);
    };

    // 1. Setup tables
    exec("CREATE TABLE A (id INTEGER, val INTEGER);");
    exec("CREATE TABLE B (a_id INTEGER, type INTEGER);");

    // Insert data into A
    exec("INSERT INTO A VALUES (1, 100);");
    exec("INSERT INTO A VALUES (2, 200);");
    exec("INSERT INTO A VALUES (3, 300);");

    // Insert data into B
    exec("INSERT INTO B VALUES (1, 10);");
    exec("INSERT INTO B VALUES (2, 20);");
    exec("INSERT INTO B VALUES (2, 20);"); // Duplicate for dedup checking
    exec("INSERT INTO B VALUES (4, 40);");

    std::cout << "Testing Basic Equi-Join..." << std::endl;
    auto res1 = exec("SELECT * FROM A JOIN B ON A.id = B.a_id;");
    ASSERT_JOIN(res1.success);
    ASSERT_JOIN(res1.rows.size() == 3); // (1,1), (2,2), (2,2)

    std::cout << "Testing Theta-Only Join..." << std::endl;
    auto res2 = exec("SELECT * FROM A JOIN B ON A.val > B.type;");
    ASSERT_JOIN(res2.success);
    ASSERT_JOIN(res2.rows.size() == 12); // Cross product (3 * 4) since all A.val (100+) > B.type (10-40)

    std::cout << "Testing Mixed AND (order independent)..." << std::endl;
    auto res3 = exec("SELECT * FROM A JOIN B ON A.id = B.a_id AND A.val = 200;");
    ASSERT_JOIN(res3.success);
    ASSERT_JOIN(res3.rows.size() == 2); // only id=2 matches, B has two rows for a_id=2

    auto res4 = exec("SELECT * FROM A JOIN B ON A.val = 200 AND A.id = B.a_id;");
    ASSERT_JOIN(res4.success);
    ASSERT_JOIN(res4.rows.size() == 2);

    std::cout << "Testing OR with Mixed Conditions (Dedup)..." << std::endl;
    // id=1 OR val=200
    auto res5 = exec("SELECT * FROM A JOIN B ON A.id = B.a_id AND A.val = 100 OR A.val = 200 AND A.id = B.a_id;");
    ASSERT_JOIN(res5.success);
    ASSERT_JOIN(res5.rows.size() == 3); 

    std::cout << "Testing Empty Table Join..." << std::endl;
    exec("CREATE TABLE C (id INTEGER);");
    auto res6 = exec("SELECT * FROM A JOIN C ON A.id = C.id;");
    ASSERT_JOIN(res6.success);
    ASSERT_JOIN(res6.rows.size() == 0);

    std::cout << "JOIN test summary: " << passed_count << " passed, " << failed_count << " failed.\n";
}
