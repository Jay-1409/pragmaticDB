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
    
    // ensure a fresh backing file
    std::remove("data.db");

    DiskManager dm;
    BufferPoolManager bpm(3, &dm);

    // Test NewPage -> write -> unpin -> flush -> delete -> FetchPage reload
    page_id_t pid1;
    Page* p1 = bpm.NewPage(&pid1);
    expect(p1 != nullptr && pid1 >= 0, "NewPage returns non-null id");
    if (p1) {
        std::memset(p1->data, 'A', PAGE_SIZE);
        p1->is_dirty = true;
        expect(p1->pin_count == 1, "NewPage returns pinned page");
        expect(bpm.UnpinPage(pid1, true), "UnpinPage after NewPage (dirty) returns true");
        expect(bpm.FlushPage(pid1), "FlushPage returns true");
        expect(bpm.DeletePage(pid1), "DeletePage returns true (page unpinned)");
        Page* fetched = bpm.FetchPage(pid1);
        expect(fetched != nullptr, "FetchPage reloads page from disk");
        if (fetched) {
            expect(fetched->data[0] == 'A' && fetched->data[PAGE_SIZE - 1] == 'A', "Fetched data matches pattern (start/end)");
            expect(bpm.UnpinPage(pid1, false), "Unpin fetched page returns true");
        }
    }

    // Unpin non-existent page should fail
    expect(!bpm.UnpinPage(static_cast<page_id_t>(999999), false), "UnpinPage on non-existent page returns false");
    // Double-unpin should fail: fetch then unpin twice
    Page* p2 = bpm.FetchPage(pid1);
    if (p2) {
        expect(bpm.UnpinPage(pid1, false), "First UnpinPage after FetchPage returns true");
        expect(!bpm.UnpinPage(pid1, false), "Second UnpinPage returns false (already unpinned)");
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
