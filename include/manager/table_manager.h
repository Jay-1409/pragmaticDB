#pragma once

#include "manager/record_manager.h"
#include "type/tuple.h"
#include "catalog/schema.h"
#include "utils/table_iterator.h"

class TableManager {
public:
    TableManager() = default;

    /**
     * @brief Inserts a tuple into the database.
     */
    RecordId InsertTuple(const Tuple& tuple);

    /**
     * @brief Deletes a tuple by its RecordId.
     */
    bool DeleteTuple(const RecordId& rid);

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
