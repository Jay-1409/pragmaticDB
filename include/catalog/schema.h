#pragma once

#include <vector>
#include <cstdint>
#include "column.h"

class Schema {
public:
    /**
     * @brief Constructs a Schema from a list of columns.
     * You should iterate over the columns to calculate the overall length_ of a tuple
     * and populate the offsets_ array so that each column knows its starting byte.
     */
    Schema(const std::vector<Column>& columns);

    const std::vector<Column>& GetColumns() const;
    const Column& GetColumn(uint32_t col_idx) const;
    uint32_t GetColOffset(uint32_t col_idx) const;
    uint32_t GetLength() const;
    uint32_t GetColumnCount() const;

private:
    uint32_t length_;
    std::vector<Column> columns_;
    std::vector<uint32_t> offsets_;
};

/* it will have multiple number of column class*/
