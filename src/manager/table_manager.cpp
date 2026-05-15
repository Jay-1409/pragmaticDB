#include "manager/table_manager.h"

RecordId TableManager::InsertTuple(const Tuple& tuple) {
    return record_manager_.Insert(tuple.GetData(), tuple.GetLength());
}

Tuple TableManager::GetTuple(const RecordId& rid, const Schema& schema) {
    char buffer[4096]; 
    uint16_t size = 0;
    if (record_manager_.Get(rid, buffer, &size)) {
        return Tuple(buffer, size);
    }
    return Tuple(); 
}

/**
TODO:
TableManager::GetTuple(RecordId) requires you to know the exact physical disk location (Page ID and Slot ID) of the row you want to read. That isn't how databases work in practice! When a user types SELECT * FROM users;, the database needs to scan the table. What we'd build: A TableIterator class that acts like a C++ iterator. It will start at the first page of the TableManager, read the first tuple, and when you call ++iterator, it automatically jumps to the next valid slot (or next page) until the end of the table is reached.
*/
