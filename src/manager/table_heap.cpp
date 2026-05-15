#include "manager/table_heap.h"

RecordId TableHeap::InsertTuple(const Tuple& tuple) {
    return record_manager_.Insert(tuple.GetData(), tuple.GetLength());
}

Tuple TableHeap::GetTuple(const RecordId& rid, const Schema& schema) {
    char buffer[4096]; 
    uint16_t size = 0;
    if (record_manager_.Get(rid, buffer, &size)) {
        return Tuple(buffer, size);
    }
    return Tuple(); 
}
