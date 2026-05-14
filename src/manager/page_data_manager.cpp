#include<iostream>
#include "ds/page.h"
#include "ds/page_header.h"
#include "manager/page_data_manager.h"
void PageDataManager::test(){
    std::cout << "Page data manager is responding" << std::endl;
}
Slot* PageDataManager::fetchSlot(uint16_t slot_id, Page* page) {
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    if(slot_id >= header->slot_count) {
        return nullptr;
    }
    return reinterpret_cast<Slot*>(page->data + sizeof(PageHeader) + slot_id * sizeof(Slot));
}
bool PageDataManager::InitializePage(Page* page, page_id_t page_id) {
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    header->page_id = page_id;
    header->free_space_offset = PAGE_SIZE - 1;
    header->slot_count = 0;
    return true;
}
bool PageDataManager::HasEnoughSpace(Page* page, uint16_t tuple_size) {
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    /** need to add a tupe and a slot  */
    uint16_t required_space = tuple_size + sizeof(Slot);
    uint16_t available_space = (header->free_space_offset + 1) - (sizeof(PageHeader) + header->slot_count * sizeof(Slot));
    return required_space <= available_space;
}
bool PageDataManager::InsertTuple(Page* page, const char* tuple_data, uint16_t tuple_size, uint16_t* slot_id) {
    if(!HasEnoughSpace(page, tuple_size)) {
        return false;
    }
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    uint16_t new_free_space_offset = header->free_space_offset - tuple_size + 1;
    /** copy data into page, from left to right */
    std::memcpy(page->data + new_free_space_offset, tuple_data, tuple_size);
    Slot page_data{new_free_space_offset, tuple_size};
    /** Insert the new slot */
    std::memcpy(page->data + sizeof(PageHeader) + header->slot_count * sizeof(Slot), &page_data, sizeof(Slot));
    *slot_id = header->slot_count;
    header->slot_count++;
    header->free_space_offset = new_free_space_offset - 1;
    return true;
}
bool PageDataManager::GetTuple(Page* page, uint16_t slot_id, char* tuple_data, uint16_t* tuple_size) {
    /**
     * Page -> input
     * slot_id -> input
     * tuple_data -> output
     * tuple_size -> output
     */
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    if(slot_id >= header->slot_count) {
        // slot id starts from 0 
        return false;
    }
    Slot* page_data = reinterpret_cast<Slot*>(page->data + sizeof(PageHeader) + slot_id * sizeof(Slot));
    std::memcpy(tuple_data, page->data + page_data->offset, page_data->length);
    *tuple_size = page_data->length;
    return true;
}

bool PageDataManager::DeleteTuple(Page* page, uint16_t slot_id) {
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    if(slot_id >= header->slot_count) {
        return false;
    }
    /** we can just mark the slot as deleted by setting its length to 0 and offset to -1 */
    Slot* s = reinterpret_cast<Slot*>(page->data + sizeof(PageHeader) + slot_id * sizeof(Slot));
    s->length = 0;
    /** 
     * Right now the space occupied by the deleted tuple remains occupied, 
     * later on a compaction is ran.
     */
    return true;
}

void PageDataManager::CompactOnePage(Page* page) {
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    int32_t write_ptr = static_cast<int32_t>(PAGE_SIZE) - 1;
    for (uint16_t i = 0; i < header->slot_count; ++i) {
        Slot* slot = fetchSlot(i, page);
        if (slot->length == 0) {
            continue;
        }
        int32_t new_offset = write_ptr - slot->length + 1;
        std::memmove(page->data + new_offset,
                     page->data + slot->offset,
                     slot->length);
        slot->offset = static_cast<uint16_t>(new_offset);
        write_ptr = new_offset - 1;
    }
    header->free_space_offset = static_cast<uint16_t>(write_ptr);
}
