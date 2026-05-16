#pragma once

#include <string>
#include "type/value.h"
#include "type/type_id.h"

/**
 * @brief Factory for converting between raw strings and Value objects.
 * 
 * Add a new case here whenever a new TypeId is introduced — the rest of
 * the codebase (Executor, etc.) will automatically support the new type.
 */
class ValueFactory {
public:
    /**
     * @brief Convert a raw string from SQL into a typed Value.
     * Used by ExecuteInsert: "42" + INTEGER → Value holding int32_t(42)
     */
    static Value FromString(const std::string& raw, TypeId type);

    /**
     * @brief Convert a typed Value into a human-readable string.
     * Used by ExecuteSelect: Value(42) + INTEGER → "42"
     */
    static std::string ToString(const Value& val, TypeId type);
};
