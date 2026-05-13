#include <cstdio>
#include <cstring>
#include <iostream>
#include "../include/tests.h"
#include "../include/record_manager.h"

void test::TestRecordManager() {
    int passed = 0, failed = 0;
    auto expect = [&](bool cond, const char *name) {
        if (cond) { std::cout << "[PASS] " << name << std::endl; ++passed; }
        else { std::cout << "[FAIL] " << name << std::endl; ++failed; }
    };

    std::remove("data.db");
    RecordManager rm;

    const char *data1 = "hello";
    RecordId rid1 = rm.Insert(data1, std::strlen(data1));
    expect(rid1.page_id != INVALID_PAGE_ID, "Insert returns valid record id");

    const char *data2 = "hi";
    bool update_small = rm.Update(rid1, data2, std::strlen(data2));
    expect(update_small, "Update with smaller payload succeeds");

    bool delete_ok = rm.Delete(rid1);
    expect(delete_ok, "Delete existing record succeeds");

    bool update_deleted = rm.Update(rid1, "x", 1);
    expect(!update_deleted, "Update on deleted record fails");

    RecordId rid2 = rm.Insert("world", 5);
    expect(rid2.page_id != INVALID_PAGE_ID, "Insert after delete succeeds");

    RecordId invalid = rm.Insert("", 0);
    expect(invalid.page_id == INVALID_PAGE_ID, "Insert with size 0 fails");

    RecordId bad_rid{INVALID_PAGE_ID, 0};
    expect(!rm.Delete(bad_rid), "Delete on invalid record id fails");
    expect(!rm.Update(bad_rid, "x", 1), "Update on invalid record id fails");

    std::cout << "\nRecordManager test summary: " << passed << " passed, "
              << failed << " failed." << std::endl;
}
