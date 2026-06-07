#pragma once
#include <string>
#include <memory>

// Expression type tags
enum class ExpressionType {
    COLUMN_REF,       // e.g. "users.id"
    CONSTANT,         // e.g. "42", "true"
    COMPARISON,       // =, >, <, >=, <=, !=
    LOGICAL           // AND, OR
};

enum class ComparisonType { EQ, NEQ, LT, GT, LTE, GTE };
enum class LogicType { AND, OR };

// Base class — all expression nodes inherit from this
struct Expression {
    ExpressionType expr_type;
    virtual ~Expression() = default;

protected:
    Expression(ExpressionType type) : expr_type(type) {}
};

// Leaf: references a column like "users.id"
struct ColumnRefExpression : Expression {
    std::string table_name;  // "users" (from "users.id")
    std::string col_name;    // "id"

    ColumnRefExpression(std::string table, std::string col)
        : Expression(ExpressionType::COLUMN_REF), table_name(std::move(table)), col_name(std::move(col)) {}
};

// Leaf: a literal value like 42 or true
struct ConstantExpression : Expression {
    std::string raw_value;   // stored as string, converted at eval time

    ConstantExpression(std::string val)
        : Expression(ExpressionType::CONSTANT), raw_value(std::move(val)) {}
};

// Internal node: left <op> right
struct ComparisonExpression : Expression {
    ComparisonType comp_type;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    ComparisonExpression(ComparisonType comp, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : Expression(ExpressionType::COMPARISON), comp_type(comp), left(std::move(l)), right(std::move(r)) {}
};

// Internal node: left AND/OR right
struct LogicalExpression : Expression {
    LogicType logic_type;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    LogicalExpression(LogicType logic, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : Expression(ExpressionType::LOGICAL), logic_type(logic), left(std::move(l)), right(std::move(r)) {}
};
