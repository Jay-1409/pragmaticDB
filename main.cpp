#include "disk_manager.h"
#include "buffer_pool_manager.h"
#include<iostream>
#include <cstdio>
#include <cstring>
#include <string>
void test_responsiveness(DiskManager& dm, BufferPoolManager& bpm) {
    dm.test();
    bpm.test();
}
void test_buffer_pool_manager() {
    int passed = 0, failed = 0;
    auto expect = [&](bool cond, const char *name) {
        if (cond) { std::cout << "[PASS] " << name << std::endl; ++passed; }
        else { std::cout << "[FAIL] " << name << std::endl; ++failed; }
    };

    // start with a fresh backing file
    std::remove("data.db");

    DiskManager dm;
    BufferPoolManager bpm(3, &dm);

    // 1) Create a page, write pattern, unpin and flush
    page_id_t pid1;
    Page* p1 = bpm.NewPage(&pid1);
    expect(p1 != nullptr && pid1 >= 0, "NewPage returns non-null id");
    if (p1) {
        std::memset(p1->data, 'A', PAGE_SIZE);
        p1->is_dirty = true;
        expect(p1->pin_count == 1, "NewPage returns pinned page");
        expect(bpm.UnpinPage(pid1, true), "UnpinPage after NewPage (dirty) returns true");
        expect(bpm.FlushPage(pid1), "FlushPage returns true");
    }

    // 2) Fetch the same page and verify contents
    Page* fetched = bpm.FetchPage(pid1);
    expect(fetched != nullptr, "FetchPage returns a page pointer");
    if (fetched) {
        expect(fetched->data[0] == 'A' && fetched->data[PAGE_SIZE - 1] == 'A', "Fetched data matches pattern (start/end)");
        expect(bpm.UnpinPage(pid1, false), "Unpin fetched page returns true");
    }

    // 3) Unpin non-existent page should return false
    expect(!bpm.UnpinPage(static_cast<page_id_t>(-12345), false), "UnpinPage on invalid page id returns false");

    // 4) Double-unpin protection: fetch then unpin twice
    Page* p2 = bpm.FetchPage(pid1);
    if (p2) {
        bool first = bpm.UnpinPage(pid1, false);
        bool second = bpm.UnpinPage(pid1, false);
        expect(first && !second, "First UnpinPage succeeds and second fails (already unpinned)");
    }

    // 5) Delete a page: allocate new, unpin, delete
    page_id_t pid2;
    Page* p3 = bpm.NewPage(&pid2);
    expect(p3 != nullptr, "NewPage for delete test returns non-null");
    if (p3) {
        expect(bpm.UnpinPage(pid2, false), "UnpinPage before DeletePage returns true");
        expect(bpm.DeletePage(pid2), "DeletePage returns true for unpinned page");
    }

    std::cout << "\nTest summary: " << passed << " passed, " << failed << " failed." << std::endl;
}
int main() {
    DiskManager dm;
    BufferPoolManager bpm(10, &dm);
    test_responsiveness(dm, bpm);
    test_buffer_pool_manager();
    return 0;
}
