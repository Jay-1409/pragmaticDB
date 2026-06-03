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

    // ── NEW: Check for optional JOIN clause ──
    std::streampos pos = ss.tellg();
    std::string next;
    if (ss >> next) {
        std::string next_upper = next;
        for (auto& c : next_upper) c = toupper(c);
        if (next_upper == "JOIN") {
            ss >> stmt->join_table_name;
            std::string on_kw;
            if (ss >> on_kw) {
                for (auto& c : on_kw) c = toupper(c);
                if (on_kw == "ON") {
                    stmt->join_condition = ParseExpression(ss);
                }
            }
        } else {
            // Not a JOIN, restore stream (might be just a semicolon)
            ss.clear();
            ss.seekg(pos);
        }
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

std::unique_ptr<Expression> Parser::ParseExpression(std::istringstream& ss) {
    auto left = ParseAndExpression(ss);
    
    while (true) {
        std::streampos pos = ss.tellg();
        std::string next;
        if (!(ss >> next)) break;
        std::string next_upper = next;
        for (auto& c : next_upper) c = toupper(c);
        
        if (next_upper == "OR") {
            auto right = ParseAndExpression(ss);
            left = std::make_unique<LogicalExpression>(LogicType::OR, std::move(left), std::move(right));
        } else {
            ss.clear();
            ss.seekg(pos);
            break;
        }
    }
    return left;
}

std::unique_ptr<Expression> Parser::ParseAndExpression(std::istringstream& ss) {
    auto left = ParseComparison(ss);
    
    while (true) {
        std::streampos pos = ss.tellg();
        std::string next;
        if (!(ss >> next)) break;
        std::string next_upper = next;
        for (auto& c : next_upper) c = toupper(c);
        
        if (next_upper == "AND") {
            auto right = ParseComparison(ss);
            left = std::make_unique<LogicalExpression>(LogicType::AND, std::move(left), std::move(right));
        } else {
            ss.clear();
            ss.seekg(pos);
            break;
        }
    }
    return left;
}

std::unique_ptr<Expression> Parser::ParseComparison(std::istringstream& ss) {
    auto left = ParseAtom(ss);
    
    std::streampos pos = ss.tellg();
    std::string op;
    if (ss >> op) {
        ComparisonType comp_type;
        bool is_comp = true;
        if (op == "=") comp_type = ComparisonType::EQ;
        else if (op == "!=") comp_type = ComparisonType::NEQ;
        else if (op == "<") comp_type = ComparisonType::LT;
        else if (op == ">") comp_type = ComparisonType::GT;
        else if (op == "<=") comp_type = ComparisonType::LTE;
        else if (op == ">=") comp_type = ComparisonType::GTE;
        else is_comp = false;
        
        if (is_comp) {
            auto right = ParseAtom(ss);
            return std::make_unique<ComparisonExpression>(comp_type, std::move(left), std::move(right));
        } else {
            ss.clear();
            ss.seekg(pos);
        }
    }
    return left;
}

std::unique_ptr<Expression> Parser::ParseAtom(std::istringstream& ss) {
    std::string token;
    
    if (!(ss >> token)) return nullptr;
    
    if (token == "(") {
        auto expr = ParseExpression(ss);
        std::string close;
        ss >> close; // consume ")"
        return expr;
    }
    
    // Strip trailing ';' or ')' if attached
    while (!token.empty() && (token.back() == ';' || token.back() == ')')) {
        token.pop_back();
    }
    
    // Strip leading '(' if attached
    if (!token.empty() && token.front() == '(') {
        token.erase(0, 1);
    }
    
    size_t dot_pos = token.find('.');
    if (dot_pos != std::string::npos) {
        return std::make_unique<ColumnRefExpression>(token.substr(0, dot_pos), token.substr(dot_pos + 1));
    }
    
    std::string token_upper = token;
    for (auto& c : token_upper) c = toupper(c);
    if (token_upper == "TRUE" || token_upper == "FALSE" || (token[0] >= '0' && token[0] <= '9') || (token[0] == '-' && token.size() > 1)) {
        return std::make_unique<ConstantExpression>(token);
    }
    
    return std::make_unique<ColumnRefExpression>("", token);
}
