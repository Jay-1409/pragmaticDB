#include<iostream>
#include<./ds/page_header.h>
#include<./ds/page.h>
#include "../include/page_data_manager.h"
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
