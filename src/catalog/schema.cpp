#include "catalog/schema.h"

Schema::Schema(const std::vector<Column>& columns) : columns_(columns) {
    length_ = 0;
    for (const auto& col : columns_) {
        offsets_.push_back(length_);
        length_ += col.GetLength();
    }
}

const std::vector<Column>& Schema::GetColumns() const { return columns_; }

const Column& Schema::GetColumn(uint32_t col_idx) const { 
    return columns_[col_idx]; 
}

uint32_t Schema::GetColOffset(uint32_t col_idx) const { 
    return offsets_[col_idx]; 
}

uint32_t Schema::GetLength() const { return length_; }

uint32_t Schema::GetColumnCount() const { 
    return static_cast<uint32_t>(columns_.size()); 
}
