#pragma once
#include <cstddef>
#include <deque>
#include <list>
#include <unordered_map>
#include <vector>
#include "disk_manager.h"
#include "page.h"
class BufferPoolManager {
    public:
        BufferPoolManager(std::size_t pool_size, DiskManager* disk_manager);
        Page* FetchPage(page_id_t page_id);
        Page* NewPage(page_id_t* page_id);
        bool UnpinPage(page_id_t page_id, bool is_dirty);
        bool FlushPage(page_id_t page_id);
        void FlushAllPages();
        bool DeletePage(page_id_t page_id);
        void test();
    private:
        bool GetFreeFrameOrVictim(frame_id_t* frame_id);
        void PinFrame(frame_id_t frame_id);
        void UnpinFrame(frame_id_t frame_id);
        std::size_t pool_size_;
        DiskManager* disk_manager_;
        /** we store the pages as objects, this allows us to optimize fetching nearby pages by the principle of locality */
        std::vector<Page> pages_;
        /**Faster O(1) lookups using unordered_map */
        std::unordered_map<page_id_t, frame_id_t> page_table_;
        std::deque<frame_id_t> free_list_;
        std::list<frame_id_t> lru_list_;
        std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> lru_pos_;
        page_id_t next_page_id_;
};