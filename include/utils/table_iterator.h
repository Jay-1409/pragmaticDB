#pragma once

#include "manager/record_manager.h"
#include "catalog/schema.h"
#include "type/tuple.h"

/**
 * @brief Iterates over all valid (non-deleted) tuples in a table sequentially.
 * 
 * Holds a RecordId cursor (page_id + slot_id). On each ++, it advances to
 * the next valid slot, skipping deleted entries (slot.length == 0) and
 * moving to the next page when the current one is exhausted.
 * 
 * Comparing to End() tells you when the scan is complete.
 */
class TableIterator {
public:
    /**
     * @brief Constructs an iterator starting at the given RecordId.
     * Used by TableManager::Begin() and TableManager::End().
     */
    TableIterator(RecordId rid, RecordManager* record_manager, const Schema& schema, size_t page_idx);

    /**
     * @brief Dereferences the iterator to get the current Tuple.
     * Implement this to call record_manager_->Get() on rid_ and return the Tuple.
     */
    Tuple operator*() const;

    /**
     * @brief Advances the iterator to the next valid (non-deleted) slot.
     * Implement this to increment slot_id, skip deleted slots (length == 0),
     * and move to the next page when slot_id >= slot_count.
     * Sets rid_ to {INVALID_PAGE_ID, 0} when exhausted (becomes End).
     */
    TableIterator& operator++();

    /**
     * @brief Compares two iterators for equality (used to check against End()).
     */
    bool operator==(const TableIterator& other) const;
    bool operator!=(const TableIterator& other) const;

private:
    RecordId rid_;
    RecordManager* record_manager_;
    Schema schema_;
    size_t page_idx_;  // current index into record_manager_->GetPageIds()
};
