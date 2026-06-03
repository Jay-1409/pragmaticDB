#pragma once

#include <vector>
#include <string>
#include "../ds/record_id.h"

class IndexProvider {
public:
    virtual ~IndexProvider() = default;

    // Given a table name, column name, and a value, return all RecordIds matching col=val
    virtual std::vector<RecordId> LookupEquals(
        const std::string& table_name,
        const std::string& col_name,
        const std::string& value) const = 0;
};

class NullIndexProvider : public IndexProvider {
public:
    std::vector<RecordId> LookupEquals(
        const std::string&, const std::string&, const std::string&) const override {
        return {}; // always returns empty
    }
};
