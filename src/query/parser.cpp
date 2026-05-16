#include <sstream>
#include "query/parser.h"

// Implement your Parser methods here!

std::unique_ptr<Statement> Parser::Parse(const std::string& sql) {
    std::istringstream ss(sql);
    std::string keyword;
    ss >> keyword;

    // Convert to uppercase for case-insensitive matching
    for (auto& c : keyword) c = toupper(c);

    if (keyword == "CREATE") return ParseCreate(ss);
    if (keyword == "INSERT") return ParseInsert(ss);
    if (keyword == "SELECT") return ParseSelect(ss);

    return nullptr; // Unknown statement
}

std::unique_ptr<Statement> Parser::ParseCreate(std::istringstream& ss) {
    // TODO: implement
    return nullptr;
}

std::unique_ptr<Statement> Parser::ParseInsert(std::istringstream& ss) {
    // TODO: implement
    return nullptr;
}

std::unique_ptr<Statement> Parser::ParseSelect(std::istringstream& ss) {
    // TODO: implement
    return nullptr;
}
