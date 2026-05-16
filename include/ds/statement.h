#pragma once

#include <string>
#include <vector>
#include "catalog/column.h"

// ── Statement type tag ────────────────────────────────────────────────────────
enum class StatementType {
    CREATE_TABLE,
    INSERT,
    SELECT
};

// ── Base class ────────────────────────────────────────────────────────────────
struct Statement {
    StatementType type;
    explicit Statement(StatementType t) : type(t) {}
    virtual ~Statement() = default;
};

// ── CREATE TABLE users (id INTEGER, is_active BOOLEAN); ──────────────────────
struct CreateTableStatement : public Statement {
    std::string table_name;
    std::vector<Column> columns;

    CreateTableStatement() : Statement(StatementType::CREATE_TABLE) {}
};

// ── INSERT INTO users VALUES (42, true); ─────────────────────────────────────
struct InsertStatement : public Statement {
    std::string table_name;
    std::vector<std::string> raw_values; // e.g. {"42", "true"}

    InsertStatement() : Statement(StatementType::INSERT) {}
};

// ── SELECT * FROM users; ──────────────────────────────────────────────────────
struct SelectStatement : public Statement {
    std::string table_name;

    SelectStatement() : Statement(StatementType::SELECT) {}
};
