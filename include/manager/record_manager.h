#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "ds/record_id.h"
#include "manager/page_data_manager.h"
class RecordManager {
    public:
        RecordManager();
        bool Delete(RecordId r);
        bool Get(const RecordId& rid, char* data, uint16_t* size);
        RecordId Insert(const char* data, size_t size);
        bool Update(const RecordId& rid, const char* data, size_t size);
    private:
        static constexpr std::size_t kDefaultPoolSize = 10;
        DiskManager disk_manager_;
        BufferPoolManager buffer_pool_manager_;
        PageDataManager page_data_manager_;
        std::vector<page_id_t> page_ids_;
};

/**
 * 
 * This class will abstract page management features for higher classes by introducing a recordId, 
 * the record Id will be used to identify an record in a page, (page_id, slot_id)
 */