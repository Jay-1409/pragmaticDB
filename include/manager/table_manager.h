#pragma once

#include "manager/record_manager.h"
#include "type/tuple.h"
#include "catalog/schema.h"
#include "utils/table_iterator.h"

class TableManager {
public:
    explicit TableManager(const std::string& filename = "data.db");

    /**
     * @brief Inserts a tuple into the database.
     */
    RecordId InsertTuple(const Tuple& tuple);

    /**
     * @brief Deletes a tuple by its RecordId.
     */
    bool DeleteTuple(const RecordId& rid);

    /**
     * @brief Flushes all dirty pages for this table to disk.
     */
    void Flush() { record_manager_.Flush(); }

    /**
     * @brief Restores the page ID list for this table (used on catalog reload).
     */
    void SetPageIds(const std::vector<page_id_t>& ids) { record_manager_.SetPageIds(ids); }

    /**
     * @brief Returns the page IDs owned by this table (used by SaveCatalog).
     */
    const std::vector<page_id_t>& GetPageIds() const { return record_manager_.GetPageIds(); }

    /**
     * @brief Save/restore next_page_id so new inserts don't reuse existing page IDs.
     */
    page_id_t GetNextPageId() const { return record_manager_.GetNextPageId(); }
    void SetNextPageId(page_id_t id) { record_manager_.SetNextPageId(id); }

    /**
     * @brief Retrieves a tuple from the database.
     */
    Tuple GetTuple(const RecordId& rid, const Schema& schema);

    /**
     * @brief Returns an iterator pointing to the first valid tuple.
     */
    TableIterator Begin(const Schema& schema);

    /**
     * @brief Returns a sentinel iterator representing end-of-table.
     */
    TableIterator End(const Schema& schema);

private:
    RecordManager record_manager_;
};
