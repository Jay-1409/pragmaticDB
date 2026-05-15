#include "catalog/column.h"

Column::Column(std::string column_name, TypeId type)
    : column_name_(std::move(column_name)), type_id_(type) {
    if (type == TypeId::INTEGER) {
        length_ = 4;
    } else if (type == TypeId::BOOLEAN) {
        length_ = 1;
    } else {
        length_ = 0; // INVALID
    }
}

std::string Column::GetName() const { return column_name_; }
TypeId Column::GetType() const { return type_id_; }
uint32_t Column::GetLength() const { return length_; }
