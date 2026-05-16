#pragma once

#include <string>
#include <vector>
#include "catalog/column.h"

// ── Statement type tag ────────────────────────────────────────────────────────
enum class StatementType {
    CREATE_TABLE,
    INSERT,
    SELECT,
    COMMIT,
    DELETE
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

// ── COMMIT; ───────────────────────────────────────────────────────────────────
// Flushes all dirty buffer pool pages to disk immediately.
struct CommitStatement : public Statement {
    CommitStatement() : Statement(StatementType::COMMIT) {}
};

// ── DELETE FROM users [WHERE col = val]; ─────────────────────────────────────
struct DeleteStatement : public Statement {
    std::string table_name;
    std::string where_column; // empty = delete all rows
    std::string where_value;

    DeleteStatement() : Statement(StatementType::DELETE) {}
};
