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

Schema Schema::Merge(
    const Schema& left,  const std::string& left_table,
    const Schema& right, const std::string& right_table
) {
    std::vector<Column> merged_cols;
    for (const auto& col : left.GetColumns()) {
        merged_cols.emplace_back(left_table + "." + col.GetName(), col.GetType());
    }
    for (const auto& col : right.GetColumns()) {
        merged_cols.emplace_back(right_table + "." + col.GetName(), col.GetType());
    }
    return Schema(merged_cols);
}
