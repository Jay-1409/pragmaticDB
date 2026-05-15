#include "utils/table_iterator.h"
#include "ds/page_header.h"
#include "ds/slot.h"

TableIterator::TableIterator(RecordId rid, RecordManager* record_manager, const Schema& schema, size_t page_idx)
    : rid_(rid), record_manager_(record_manager), schema_(schema), page_idx_(page_idx) {}

// Dereference: fetch the tuple at the current RecordId
Tuple TableIterator::operator*() const {
    char buffer[4096];
    uint16_t size = 0;
    record_manager_->Get(rid_, buffer, &size);
    return Tuple(buffer, size);
}

// Advance to the next valid (non-deleted) slot
TableIterator& TableIterator::operator++() {
    const auto& page_ids = record_manager_->GetPageIds();
    
    // Try the next slot on the current page
    rid_.slot_id++;

    while (true) {
        uint16_t slot_count = record_manager_->GetSlotCount(rid_.page_id);

        // Walk slots on this page looking for a valid (non-deleted) one
        while (rid_.slot_id < slot_count) {
            char buffer[4096];
            uint16_t size = 0;
            if (record_manager_->Get(rid_, buffer, &size)) {
                return *this; // Found a valid slot!
            }
            rid_.slot_id++; // This slot was deleted, skip it
        }

        // Exhausted this page — move to the next one
        page_idx_++;
        if (page_idx_ >= page_ids.size()) {
            // No more pages — we are at End()
            rid_ = {INVALID_PAGE_ID, 0};
            return *this;
        }
        rid_.page_id = page_ids[page_idx_];
        rid_.slot_id = 0;
    }
}

bool TableIterator::operator==(const TableIterator& other) const {
    return rid_.page_id == other.rid_.page_id && rid_.slot_id == other.rid_.slot_id;
}

bool TableIterator::operator!=(const TableIterator& other) const {
    return !(*this == other);
}