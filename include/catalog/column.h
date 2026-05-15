#pragma once

#include <string>
#include <cstdint>
#include "type/type_id.h"

class Column {
public:
    /**
     * @brief Constructor for a new Column.
     * Implement this to automatically set the length_ to 4 for INTEGER and 1 for BOOLEAN.
     */
    Column(std::string column_name, TypeId type);
    std::string GetName() const;
    TypeId GetType() const;
    uint32_t GetLength() const;

private:
    std::string column_name_;
    TypeId type_id_;
    uint32_t length_;
};
