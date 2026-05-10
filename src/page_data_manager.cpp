#include<iostream>
#include "page.h"
#include "page_header.h"
#include "page_data_manager.h"
void PageDataManager::test(){
    std::cout << "Page data manager is responding" << std::endl;
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
    uint16_t required_space = tuple_size + sizeof(PageData);
    uint16_t available_space = header->free_space_offset - (sizeof(PageHeader) + header->slot_count * sizeof(PageData));
    return required_space <= available_space;
}
bool PageDataManager::InsertTuple(Page* page, const char* tuple_data, uint16_t tuple_size, uint16_t* slot_id) {
    if(!HasEnoughSpace(page, tuple_size)) {
        return false;
    }
    PageHeader* header = reinterpret_cast<PageHeader*>(page->data);
    uint16_t new_free_space_offset = header->free_space_offset - tuple_size;
    /** copy data into page, from left to right */
    std::memcpy(page->data + new_free_space_offset, tuple_data, tuple_size);
    PageData page_data{new_free_space_offset, tuple_size};
    /** Insert the new slot */
    std::memcpy(page->data + sizeof(PageHeader) + header->slot_count * sizeof(PageData), &page_data, sizeof(PageData));
    *slot_id = header->slot_count;
    header->slot_count++;
    header->free_space_offset = new_free_space_offset;
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
    PageData* page_data = reinterpret_cast<PageData*>(page->data + sizeof(PageHeader) + slot_id * sizeof(PageData));
    std::memcpy(tuple_data, page->data + page_data->offset, page_data->length);
    *tuple_size = page_data->length;
    return true;
}
