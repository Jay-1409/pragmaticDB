#pragma once

#include "manager/record_manager.h"
#include "type/tuple.h"

class TableHeap {
public:
    TableHeap() = default;

    /**
     * @brief Inserts a tuple into the database.
     * Implement this to get the raw data and size from the tuple,
     * pass it to record_manager_.Insert(), and return the generated RecordId.
     */
    RecordId InsertTuple(const Tuple& tuple);

    /**
     * @brief Retrieves a tuple from the database.
     * Implement this to fetch the raw bytes from record_manager_.Get(),
     * and construct a new Tuple object from those bytes.
     */
    Tuple GetTuple(const RecordId& rid, const Schema& schema);

private:
    RecordManager record_manager_;
};
