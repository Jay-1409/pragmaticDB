#pragma once
#include<iostream>
#include "../src/ds/record_id.h"
class RecordManager {
    public:
        RecordManager();
        bool Delete(RecordId r);
        RecordId Insert(const char* data, size_t size);
        bool Update(const RecordId& rid, const char* data, size_t size);
    private:
};

/**
 * 
 * This class will abstract page management features for higher classes by introducing a recordId, 
 * the record Id will be used to identify an record in a page, (page_id, slot_id)
 */