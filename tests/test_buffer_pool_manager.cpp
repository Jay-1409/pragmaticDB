#include <iostream>
#include <cstdio>
#include <cstring>
#include "../include/tests.h"

void test::TestBufferPoolManager() {
    int passed = 0, failed = 0;
    auto expect = [&](bool cond, const char *name) {
        if (cond) { std::cout << "[PASS] " << name << std::endl; ++passed; }
        else { std::cout << "[FAIL] " << name << std::endl; ++failed; }
    };
    auto fill = [&](Page* page, char value) {
        if (!page) {
            return;
        }
        std::memset(page->data, value, PAGE_SIZE);
        page->is_dirty = true;
    };
    auto matches = [&](const Page* page, char value) {
        if (!page) {
            return false;
        }
        return page->data[0] == value && page->data[PAGE_SIZE - 1] == value;
    };

    // start with a fresh backing file
    std::remove("data.db");

    // 1) Create, write, unpin, flush, and refetch
    {
        DiskManager dm;
        BufferPoolManager bpm(3, &dm);
        page_id_t pid1;
        Page* p1 = bpm.NewPage(&pid1);
        expect(p1 != nullptr && pid1 >= 0, "NewPage returns non-null id");
        if (p1) {
            fill(p1, 'A');
            expect(p1->pin_count == 1, "NewPage returns pinned page");
            expect(bpm.UnpinPage(pid1, true), "UnpinPage after NewPage (dirty) returns true");
            expect(bpm.FlushPage(pid1), "FlushPage returns true");
        }

        Page* fetched = bpm.FetchPage(pid1);
        expect(fetched != nullptr, "FetchPage returns a page pointer");
        if (fetched) {
            expect(matches(fetched, 'A'), "Fetched data matches pattern (start/end)");
            expect(bpm.UnpinPage(pid1, false), "Unpin fetched page returns true");
        }
    }

    // 2) Unpin and flush invalid page ids
    {
        DiskManager dm;
        BufferPoolManager bpm(2, &dm);
        expect(!bpm.UnpinPage(static_cast<page_id_t>(-12345), false), "UnpinPage on invalid page id returns false");
        expect(!bpm.FlushPage(static_cast<page_id_t>(-12345)), "FlushPage on invalid page id returns false");
    }

    // 3) Double-unpin protection
    {
        DiskManager dm;
        BufferPoolManager bpm(2, &dm);
        page_id_t pid1;
        Page* p1 = bpm.NewPage(&pid1);
        expect(p1 != nullptr, "NewPage for double-unpin returns non-null");
        if (p1) {
            expect(bpm.UnpinPage(pid1, false), "First UnpinPage succeeds");
            expect(!bpm.UnpinPage(pid1, false), "Second UnpinPage fails (already unpinned)");
        }
    }

    // 4) Delete pinned pages should fail, unpinned should succeed
    {
        DiskManager dm;
        BufferPoolManager bpm(2, &dm);
        page_id_t pid1;
        Page* p1 = bpm.NewPage(&pid1);
        expect(p1 != nullptr, "NewPage for delete test returns non-null");
        if (p1) {
            expect(!bpm.DeletePage(pid1), "DeletePage fails for pinned page");
            expect(bpm.UnpinPage(pid1, false), "UnpinPage before DeletePage returns true");
            expect(bpm.DeletePage(pid1), "DeletePage returns true for unpinned page");
        }
    }

    // 5) All pages pinned means no eviction candidate
    {
        DiskManager dm;
        BufferPoolManager bpm(2, &dm);
        page_id_t pid1;
        page_id_t pid2;
        Page* p1 = bpm.NewPage(&pid1);
        Page* p2 = bpm.NewPage(&pid2);
        expect(p1 != nullptr && p2 != nullptr, "Two NewPage calls succeed when pool has space");
        page_id_t pid3;
        Page* p3 = bpm.NewPage(&pid3);
        expect(p3 == nullptr, "NewPage returns nullptr when all frames are pinned");
        Page* fetched = bpm.FetchPage(static_cast<page_id_t>(999));
        expect(fetched == nullptr, "FetchPage returns nullptr when all frames are pinned");
    }

    // 6) LRU eviction preserves dirty data on disk
    {
        DiskManager dm;
        BufferPoolManager bpm(2, &dm);

        page_id_t pid_a;
        page_id_t pid_b;
        Page* a = bpm.NewPage(&pid_a);
        Page* b = bpm.NewPage(&pid_b);
        expect(a != nullptr && b != nullptr, "NewPage creates two pages for LRU test");
        if (a && b) {
            fill(a, 'A');
            fill(b, 'B');
            expect(bpm.UnpinPage(pid_a, true), "Unpin page A for eviction");
            expect(bpm.UnpinPage(pid_b, true), "Unpin page B for eviction");

            Page* a_ref = bpm.FetchPage(pid_a);
            expect(a_ref != nullptr, "FetchPage returns page A");
            if (a_ref) {
                expect(bpm.UnpinPage(pid_a, false), "Unpin page A after refetch");
            }

            page_id_t pid_c;
            Page* c = bpm.NewPage(&pid_c);
            expect(c != nullptr, "NewPage succeeds by evicting LRU page");
            if (c) {
                fill(c, 'C');
                expect(bpm.UnpinPage(pid_c, true), "Unpin page C after create");
            }

            Page* b_ref = bpm.FetchPage(pid_b);
            expect(b_ref != nullptr, "FetchPage returns evicted page B from disk");
            if (b_ref) {
                expect(matches(b_ref, 'B'), "Evicted page B data matches pattern");
                expect(bpm.UnpinPage(pid_b, false), "Unpin page B after fetch");
            }
        }
    }

    // 7) FlushAllPages persists dirty data across new manager instance
    {
        DiskManager dm;
        page_id_t pid1;
        page_id_t pid2;
        {
            BufferPoolManager bpm(2, &dm);
            Page* p1 = bpm.NewPage(&pid1);
            Page* p2 = bpm.NewPage(&pid2);
            expect(p1 != nullptr && p2 != nullptr, "NewPage creates two pages for FlushAllPages");
            if (p1 && p2) {
                fill(p1, 'X');
                fill(p2, 'Y');
                expect(bpm.UnpinPage(pid1, true), "Unpin page X before flush-all");
                expect(bpm.UnpinPage(pid2, true), "Unpin page Y before flush-all");
                bpm.FlushAllPages();
            }
        }

        BufferPoolManager bpm2(1, &dm);
        Page* p1_ref = bpm2.FetchPage(pid1);
        expect(p1_ref != nullptr, "FetchPage returns page X after FlushAllPages");
        if (p1_ref) {
            expect(matches(p1_ref, 'X'), "Page X data persists after FlushAllPages");
            expect(bpm2.UnpinPage(pid1, false), "Unpin page X after fetch");
        }
        Page* p2_ref = bpm2.FetchPage(pid2);
        expect(p2_ref != nullptr, "FetchPage returns page Y after FlushAllPages");
        if (p2_ref) {
            expect(matches(p2_ref, 'Y'), "Page Y data persists after FlushAllPages");
            expect(bpm2.UnpinPage(pid2, false), "Unpin page Y after fetch");
        }
    }

    std::cout << "\nTest summary: " << passed << " passed, " << failed << " failed." << std::endl;
}