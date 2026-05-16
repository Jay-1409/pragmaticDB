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
    std::string keyword;
    ss >> keyword;
    for (auto& c : keyword) c = toupper(c);
    if (keyword != "TABLE") return nullptr;

    auto stmt = std::make_unique<CreateTableStatement>();
    ss >> stmt->table_name;

    // Read the rest of the line: e.g., "(id INTEGER, is_active BOOLEAN);"
    std::string rest;
    std::getline(ss, rest);

    // Clean up punctuation to make extraction easier
    for (char& c : rest) {
        if (c == '(' || c == ')' || c == ',' || c == ';') {
            c = ' ';
        }
    }

    std::istringstream col_ss(rest);
    std::string col_name, type_str;
    while (col_ss >> col_name >> type_str) {
        for (auto& c : type_str) c = toupper(c);
        TypeId type = TypeId::INTEGER; // default
        if (type_str == "BOOLEAN") type = TypeId::BOOLEAN;
        
        stmt->columns.emplace_back(col_name, type);
    }

    return stmt;
}

std::unique_ptr<Statement> Parser::ParseInsert(std::istringstream& ss) {
    std::string keyword;
    ss >> keyword;
    for (auto& c : keyword) c = toupper(c);
    if (keyword != "INTO") return nullptr;

    auto stmt = std::make_unique<InsertStatement>();
    ss >> stmt->table_name;

    ss >> keyword;
    for (auto& c : keyword) c = toupper(c);
    if (keyword != "VALUES") return nullptr;

    std::string rest;
    std::getline(ss, rest);

    for (char& c : rest) {
        if (c == '(' || c == ')' || c == ',' || c == ';') {
            c = ' ';
        }
    }

    std::istringstream val_ss(rest);
    std::string val;
    while (val_ss >> val) {
        stmt->raw_values.push_back(val);
    }

    return stmt;
}

std::unique_ptr<Statement> Parser::ParseSelect(std::istringstream& ss) {
    std::string keyword;
    ss >> keyword;
    if (keyword != "*") return nullptr;

    ss >> keyword;
    for (auto& c : keyword) c = toupper(c);
    if (keyword != "FROM") return nullptr;

    auto stmt = std::make_unique<SelectStatement>();
    ss >> stmt->table_name;

    // Remove trailing semicolon if present
    if (!stmt->table_name.empty() && stmt->table_name.back() == ';') {
        stmt->table_name.pop_back();
    }

    return stmt;
}
