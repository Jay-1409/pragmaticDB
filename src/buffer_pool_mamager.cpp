#include "../include/buffer_pool_manager.h"
#include <iostream>

BufferPoolManager::BufferPoolManager(std::size_t pool_size, DiskManager* disk_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager) {
        pages_.resize(pool_size);
        for (frame_id_t i = 0; i < static_cast<frame_id_t>(pool_size_); ++i) {
            free_list_.push_back(i);
            pages_[i].page_id = INVALID_PAGE_ID;
            pages_[i].pin_count = 0;
            pages_[i].is_dirty = false;
        }
        page_table_.clear();
        next_page_id_ = 0;
    }

bool BufferPoolManager::GetFreeFrameOrVictim(frame_id_t* frame_id) {
    if(free_list_.empty()) {
        if(lru_list_.empty()) {
            return false;
        }
        *frame_id = lru_list_.front();
        lru_list_.pop_front();
        lru_pos_.erase(*frame_id);
        return true;
    } 
    *frame_id = free_list_.front();
    free_list_.pop_front();
    return true;
}

void BufferPoolManager::PinFrame(frame_id_t frame_id) {
    if(lru_pos_.count(frame_id)) {
        lru_list_.erase(lru_pos_[frame_id]);
        lru_pos_.erase(frame_id);
    }
}

void BufferPoolManager::UnpinFrame(frame_id_t frame_id) {
    if(!lru_pos_.count(frame_id)) {
        lru_list_.push_back(frame_id);
        lru_pos_[frame_id] = --lru_list_.end();
    }
}

Page* BufferPoolManager::FetchPage(page_id_t page_id) {
    if(page_table_.count(page_id)) {
        frame_id_t frame_id = page_table_[page_id];
        pages_[frame_id].pin_count++;
        PinFrame(frame_id);
        return &pages_[frame_id];
    }
    frame_id_t next_frame_id; 
    if(!GetFreeFrameOrVictim(&next_frame_id)) {
        return nullptr;
    }
    if(pages_[next_frame_id].is_dirty) {
        disk_manager_->WritePage(pages_[next_frame_id].page_id, pages_[next_frame_id].data);
    }
    if(pages_[next_frame_id].page_id != INVALID_PAGE_ID) {
        page_table_.erase(pages_[next_frame_id].page_id);
    }
    page_table_[page_id] = next_frame_id;
    pages_[next_frame_id].page_id = page_id;
    pages_[next_frame_id].pin_count = 1;
    pages_[next_frame_id].is_dirty = false;
    PinFrame(next_frame_id);
    disk_manager_->ReadPage(page_id, pages_[next_frame_id].data);
    return &pages_[next_frame_id];
}

Page* BufferPoolManager::NewPage(page_id_t* page_id) {
    
}

bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
    (void)page_id;
    (void)is_dirty;
    return false;
}

bool BufferPoolManager::FlushPage(page_id_t page_id) {
    (void)page_id;
    return false;
}

void BufferPoolManager::FlushAllPages() {}

bool BufferPoolManager::DeletePage(page_id_t page_id) {
    (void)page_id;
    return false;
}

void BufferPoolManager::test() {
    std::cout << "Buffer pool manager is responding" << std::endl;
}
