#include <cstdio>
#include <cstring>
#include <iostream>
#include "tests.h"
#include "manager/record_manager.h"

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

    char buffer[64] = {};
    uint16_t out_size = 0;
    bool get_ok = rm.Get(rid1, buffer, &out_size);
    expect(get_ok, "Get returns true for existing record");
    expect(out_size == std::strlen(data1), "Get returns correct size");
    expect(std::memcmp(buffer, data1, out_size) == 0, "Get returns correct data");

    const char *data2 = "hi";
    bool update_small = rm.Update(rid1, data2, std::strlen(data2));
    expect(update_small, "Update with smaller payload succeeds");

    std::memset(buffer, 0, sizeof(buffer));
    out_size = 0;
    bool get_after_update = rm.Get(rid1, buffer, &out_size);
    expect(get_after_update, "Get after update succeeds");
    expect(out_size == std::strlen(data2), "Get after update returns correct size");
    expect(std::memcmp(buffer, data2, out_size) == 0, "Get after update returns correct data");

    bool delete_ok = rm.Delete(rid1);
    expect(delete_ok, "Delete existing record succeeds");

    std::memset(buffer, 0, sizeof(buffer));
    out_size = 0;
    bool get_deleted = rm.Get(rid1, buffer, &out_size);
    expect(!get_deleted, "Get on deleted record fails");

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
