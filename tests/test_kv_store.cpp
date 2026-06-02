#include <iostream>
#include <filesystem>
#include "tests.h"
#include "kv_store/kv_store.h"
#include "query/executor.h"
#include "query/parser.h"
#include "catalog/catalog.h"

#define ASSERT(expr) \
    if (!(expr)) { \
        std::cerr << "Assertion failed: " << #expr << " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        failed_count++; \
    } else { \
        std::cout << "Passed: " << #expr << "\n"; \
        passed_count++; \
    }

void test::TestKVStore() {
    int passed_count = 0;
    int failed_count = 0;
    std::cout << "\n=== Testing KVStore ===\n";

    const std::string kv_path = "/temp/test_kv_store.db";
    std::filesystem::remove(kv_path); // Ensure clean slate

    // Test basic Put/Get
    {
        KVStore kv(kv_path);
        kv.Put("user:1", "{\"name\":\"Alice\"}");
        bool found = false;
        std::string val = kv.Get("user:1", found);
        ASSERT(found == true);
        ASSERT(val == "{\"name\":\"Alice\"}");
    }

    // GET missing key 
    {
        KVStore kv(kv_path);
        bool found = false;
        kv.Get("does:not:exist", found);
        ASSERT(found == false);
    }

    // Test Exists
    {
        KVStore kv(kv_path);
        kv.Put("temp:key", "value");
        ASSERT(kv.Delete("temp:key") == true);
        ASSERT(kv.Exists("temp:key") == false);
        // Deleting again returns false
        ASSERT(kv.Delete("temp:key") == false);
    }

    // Overwrite existing key
    {
        KVStore kv(kv_path);
        kv.Put("counter", "1");
        kv.Put("counter", "2");
        bool found = false;
        std::string val = kv.Get("counter", found);
        ASSERT(val == "2");
        ASSERT(found == true);
    }

    // Persistence (flush -> load)
    {
        {
            KVStore kv(kv_path);
            kv.Put("session", "abc123");
            kv.Flush();
        }
        {
            KVStore kv2(kv_path);
            kv2.Load();
            bool fa = false, fb = false;
            ASSERT(kv2.Get("persist:a", fa) == "hello");
            ASSERT(fa == true);
            ASSERT(kv2.Get("persist:b", fb) == "world");
            ASSERT(fb == true);
        }
    }

    // Parser: PUT command
    {
        Parser parser;
        auto stmt = parser.Parse("PUT user:42 {\"name\":\"Jay\",\"active\":true}");
        ASSERT(stmt != nullptr);
        ASSERT(stmt->type == StatementType::KV_PUT);
        auto& kv = static_cast<KVPutStatement&>(*stmt);
        ASSERT(kv.key == "user:42");
        ASSERT(kv.value == "{\"name\":\"Jay\",\"active\":true}");
    }

    // Parser: GET command
    {
        Parser parser;
        auto stmt = parser.Parse("GET user:42");
        ASSERT(stmt != nullptr);
        ASSERT(stmt->type == StatementType::KV_GET);
        ASSERT(static_cast<KVGetStatement&>(*stmt).key == "user:42");
    }

    // Parser: EXISTS command
    {
        Parser parser;
        auto stmt = parser.Parse("EXISTS user:42");
        ASSERT(stmt != nullptr);
        ASSERT(stmt->type == StatementType::KV_EXISTS);
        ASSERT(static_cast<KVExistsStatement&>(*stmt).key == "user:42");
    }

    // Parser: DELETE FROM command
    {
        Parser parser;
        auto stmt = parser.Parse("DELETE FROM users WHERE id = 1");
        ASSERT(stmt != nullptr);
        ASSERT(stmt->type == StatementType::DELETE);
    }

    // Executor: Execute KV PUT/GET/EXISTS
    {
        Catalog catalog;
        Executor executor(catalog);
        Parser parser;

        auto put = parser.Parse("PUT product:1 sneakers");
        ASSERT(put != nullptr);
        auto r1 = executor.Execute(*put);
        ASSERT(r1.success == true);
        ASSERT(r1.message == "OK");

        auto get = parser.Parse("GET product:1");
        ASSERT(get != nullptr);
        auto r2 = executor.Execute(*get);
        ASSERT(r2.success == true);
        ASSERT(r2.message == "sneakers");

        auto exists = parser.Parse("EXISTS product:1");
        ASSERT(exists != nullptr);
        auto r3 = executor.Execute(*exists);
        ASSERT(r3.success == true);
        ASSERT(r3.message == "1");

        auto del = parser.Parse("DELETE product:1");
        ASSERT(del != nullptr);
        auto r4 = executor.Execute(*del);
        ASSERT(r4.success == true);

        auto exists2 = parser.Parse("EXISTS product:1");
        auto r5 = executor.Execute(*exists2);
        ASSERT(r5.message == "0");
    }

    // Get on missing
    {
        Catalog catalog;
        Executor executor(catalog);
        Parser parser;
        auto get = parser.Parse("GET ghost:key");
        auto r = executor.Execute(*get);
        ASSERT(r.success == false);
        ASSERT(r.message == "(nil)");
    }

    // Cleanup
    std::filesystem::remove(kv_path);

    std::cout << "KVStore tests passed: " << passed_count << ", failed: " << failed_count << "\n";
}
