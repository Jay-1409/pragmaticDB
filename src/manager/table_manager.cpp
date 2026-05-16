#include "manager/table_manager.h"

TableManager::TableManager(const std::string& filename)
    : record_manager_(filename) {}

RecordId TableManager::InsertTuple(const Tuple& tuple) {
    return record_manager_.Insert(tuple.GetData(), tuple.GetLength());
}

bool TableManager::DeleteTuple(const RecordId& rid) {
    return record_manager_.Delete(rid);
}

Tuple TableManager::GetTuple(const RecordId& rid, const Schema& schema) {
    char buffer[4096]; 
    uint16_t size = 0;
    if (record_manager_.Get(rid, buffer, &size)) {
        return Tuple(buffer, size);
    }
    return Tuple(); 
}

TableIterator TableManager::Begin(const Schema& schema) {
    const auto& page_ids = record_manager_.GetPageIds();
    if (page_ids.empty()) {
        return End(schema);
    }
    // Start at page 0, slot 0 — operator++ logic will skip deleted slots
    RecordId first{page_ids[0], 0};
    TableIterator it(first, &record_manager_, schema, 0);
    // If slot 0 is deleted, advance to the first valid one
    char buffer[4096]; uint16_t size = 0;
    if (!record_manager_.Get(first, buffer, &size)) {
        ++it;
    }
    return it;
}

TableIterator TableManager::End(const Schema& schema) {
    return TableIterator({INVALID_PAGE_ID, 0}, &record_manager_, schema, 0);
}

/**
TODO:

TableManager::GetTuple(RecordId) requires you to know the exact physical disk location (Page ID and Slot ID) of the row you want to read. 

That isn't how databases work in practice! 

When a user types SELECT * FROM users;,
the database needs to scan the table.

What we'd build: A TableIterator class that acts like a C++ iterator. It will start at the first page of the TableManager, read the first tuple, and when you call ++iterator, it automatically jumps to the next valid slot (or next page) until the end of the table is reached.
*/
