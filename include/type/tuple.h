#pragma once

#include <vector>
#include "catalog/schema.h"
#include "type/value.h"

class Tuple {
public:
    // Default constructor for an empty tuple
    Tuple() = default;

    /**
     * @brief Constructs a Tuple from a vector of Values and a Schema.
     * Implement this to resize data_ based on the schema's total length,
     * then loop over values and call SerializeToChar at the correct offset.
     */
    Tuple(std::vector<Value> values, const Schema& schema);

    /**
     * @brief Constructs a Tuple from raw bytes.
     * Implement this to copy `size` bytes from `raw_data` into `data_`.
     */
    Tuple(const char* raw_data, uint32_t size);

    /**
     * @brief Deserializes a specific column back into a Value object.
     * Implement this to find the correct byte offset, then deserialize the raw bytes.
     */
    Value GetValue(const Schema& schema, uint32_t col_idx) const;

    const char* GetData() const;
    uint32_t GetLength() const;

private:
    std::vector<char> data_;
};
