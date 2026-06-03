#include "type/tuple.h"

Tuple::Tuple(std::vector<Value> values, const Schema& schema) {
    data_.resize(schema.GetLength());
    for (size_t i = 0; i < values.size(); i++) {
        char* dest = data_.data() + schema.GetColOffset(i);
        values[i].SerializeToChar(dest);
    }
}

Value Tuple::GetValue(const Schema& schema, uint32_t col_idx) const {
    Column col = schema.GetColumn(col_idx);
    const char* src = data_.data() + schema.GetColOffset(col_idx);
    
    Value val(TypeId::INVALID);
    val.DeserializeFromChar(src, col.GetType());
    return val;
}

const char* Tuple::GetData() const {
    return data_.data();
}

uint32_t Tuple::GetLength() const {
    return static_cast<uint32_t>(data_.size());
}

Tuple::Tuple(const char* raw_data, uint32_t size) {
    data_.assign(raw_data, raw_data + size);
}

Tuple Tuple::Merge(const Tuple& left, const Tuple& right) {
    Tuple merged;
    merged.data_.reserve(left.GetLength() + right.GetLength());
    merged.data_.insert(merged.data_.end(), left.data_.begin(), left.data_.end());
    merged.data_.insert(merged.data_.end(), right.data_.begin(), right.data_.end());
    return merged;
}
