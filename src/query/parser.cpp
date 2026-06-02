#include <sstream>
#include "query/parser.h"

// Implement your Parser methods here!

std::unique_ptr<Statement> Parser::Parse(const std::string& sql) {
    std::istringstream ss(sql);
    std::string keyword;
    ss >> keyword;

    // Convert to uppercase for case-insensitive matching
    for (auto& c : keyword) c = toupper(c);

    // KV commands
    if (keyword == "PUT") return ParseKVPut(ss);
    if (keyword == "GET") return ParseKVGet(ss);
    if (keyword == "EXISTS") return ParseKVExists(ss);
    if (keyword == "DELETE") {
        // Need to peek next word to distinguish between DELETE FROM and KV DELETE
        std::string next;
        if (ss >> next) {
            std::string up = next;
            for (auto& c : up) c = toupper(c);
            if (up != "FROM") {
                auto stmt = std::make_unique<KVDeleteStatement>();
                stmt->key = next;
                return stmt;
            }
            // If it's "FROM", we fall through to SQL DELETE parsing
            std::string rest;
            std::getline(ss, rest);
            std::istringstream rebuilt(next + " " + rest);
            return ParseDelete(rebuilt);
        }
        return nullptr; // Invalid DELETE syntax
    }
    // SQL statements
    if (keyword == "CREATE") return ParseCreate(ss);
    if (keyword == "INSERT") return ParseInsert(ss);
    if (keyword == "SELECT") return ParseSelect(ss);
    if (keyword == "COMMIT") return std::make_unique<CommitStatement>();
    if (keyword == "DELETE") return ParseDelete(ss);

    return nullptr;
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

// Parses: DELETE FROM <table> [WHERE <col> = <val>];
std::unique_ptr<Statement> Parser::ParseDelete(std::istringstream& ss) {
    std::string keyword;
    ss >> keyword;
    for (auto& c : keyword) c = toupper(c);
    if (keyword != "FROM") return nullptr;

    auto stmt = std::make_unique<DeleteStatement>();
    ss >> stmt->table_name;

    // Remove trailing semicolon
    if (!stmt->table_name.empty() && stmt->table_name.back() == ';') {
        stmt->table_name.pop_back();
    }

    // Check for optional WHERE clause
    if (ss >> keyword) {
        for (auto& c : keyword) c = toupper(c);
        if (keyword == "WHERE") {
            std::string col, eq, val;
            if (ss >> col >> eq >> val) {
                stmt->where_column = col;
                // Strip trailing semicolon from value
                if (!val.empty() && val.back() == ';') val.pop_back();
                stmt->where_value = val;
            }
        }
    }

    return stmt;
}


// PUT <key> <value>
std::unique_ptr<Statement> Parser::ParseKVPut(std::istringstream& ss) {
    auto stmt = std::make_unique<KVPutStatement>();
    if (!(ss >> stmt->key >> stmt->value)) return nullptr;

    std::string value;
    if (!std::getline(ss, value)) return nullptr;

    
    if (!value.empty() && value.front() == ' ') value.erase(0, 1);
    
    if (!value.empty() && value.back() == ';') value.pop_back();

    if (value.empty()) return nullptr;
    stmt->value = value;
    return stmt;
}

// GET <key>
std::unique_ptr<Statement> Parser::ParseKVGet(std::istringstream& ss) {
    auto stmt = std::make_unique<KVGetStatement>();
    if (!(ss >> stmt->key)) return nullptr;
    if (!stmt->key.empty() && stmt->key.back() == ';') stmt->key.pop_back();
    return stmt;
}

// EXISTS <key>
std::unique_ptr<Statement> Parser::ParseKVExists(std::istringstream& ss) {
    auto stmt = std::make_unique<KVExistsStatement>();
    if (!(ss >> stmt->key)) return nullptr;
    if (!stmt->key.empty() && stmt->key.back() == ';') stmt->key.pop_back();
    return stmt;
}
