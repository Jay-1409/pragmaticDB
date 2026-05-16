#pragma once

#include <string>
#include <memory>
#include "../ds/statement.h"

/**
 * @brief Tokenizes and parses a SQL string into a Statement object.
 *
 * Supported syntax:  !!(ONLY AS OF NOW)!! 
 *   CREATE TABLE <name> (<col> <type>, ...);
 *   INSERT INTO <name> VALUES (<val>, ...);
 *   SELECT * FROM <name>;
 */
class Parser {
public:
    /**
     * @brief Parse the given SQL string.
     * @returns A heap-allocated Statement* (caller owns it), or nullptr on error.
     * Implement this to tokenize the input using std::stringstream and
     * dispatch to the correct parseCreate/parseInsert/parseSelect helper.
     */
    std::unique_ptr<Statement> Parse(const std::string& sql);

private:
    std::unique_ptr<Statement> ParseCreate(std::istringstream& ss);
    std::unique_ptr<Statement> ParseInsert(std::istringstream& ss);
    std::unique_ptr<Statement> ParseSelect(std::istringstream& ss);
    std::unique_ptr<Statement> ParseDelete(std::istringstream& ss);
};
