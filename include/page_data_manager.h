#include "../src/ds/page_header.h"
#include "../include/buffer_pool_manager.h"
#include <iostream>
#include <cstring>
#include "../src/ds/slot.h"
class PageDataManager {
public:
    PageDataManager() = default;
    bool InitializePage(Page* page, page_id_t page_id);
    bool InsertTuple(Page* page, const char* tuple_data, uint16_t tuple_size, uint16_t* slot_id);
    bool GetTuple(Page* page, uint16_t slot_id, char* tuple_data, uint16_t* tuple_size);
    bool DeleteTuple(Page* page, uint16_t slot_id);
    void test();
private:
    Slot* fetchSlot(uint16_t slot_id, Page* page);
    bool HasEnoughSpace(Page* page, uint16_t tuple_size);  
    void CompactOnePage(Page* page);
    bool CompactAllPage();
}; 


/** Notes 
 * This class is responsible for handling data within a page
 * Header structure is defined in page_header.h, which includes page_id and free_space_offset
 * free_space_offset indicates the offset from the end of the page where free space starts
 * There will be slots in the data which will start from the beginning which will store the offset and size of each tuple
 * basically they will be pointers which will point to the actual data for that slot (via the offset that is)
 * 
 * it has a slot count which indicated the index on which the next slot can be placed, slot here again is the offset and the size of the next tuple
 * 
 * We go from right to left when to comes to inserting data, but the data is inserted in left to right format only, so when you read the data you have to start from the left ro right only to get the data in the correct order. 
 * 
 * 
*/