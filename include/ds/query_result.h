#pragma once

#include <string>
#include <vector>

/**
 * @brief Holds the result of a query execution.
 *
 * success: whether the query ran without errors
 * message: human-readable status (e.g. "2 rows returned", error text)
 * rows: for SELECT, each row is a vector of string-formatted column values
 */
struct QueryResult {
    bool success = false;
    std::string message;
    std::vector<std::vector<std::string>> rows; // each inner vector = one row
};
