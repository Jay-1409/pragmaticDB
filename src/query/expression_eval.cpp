#include "query/expression_eval.h"
#include <stdexcept>
#include <string>

static int32_t GetColIdx(const Schema& schema, const std::string& col_name, const std::string& table_name) {
    const auto& cols = schema.GetColumns();
    for (size_t i = 0; i < cols.size(); ++i) {
        if (!table_name.empty()) {
            if (cols[i].GetName() == table_name + "." + col_name) {
                return i;
            }
        } else {
            if (cols[i].GetName() == col_name) {
                return i;
            }
            size_t dot_pos = cols[i].GetName().find('.');
            if (dot_pos != std::string::npos && cols[i].GetName().substr(dot_pos + 1) == col_name) {
                return i;
            }
        }
    }
    return -1;
}

static Value EvaluateAtom(const Expression* expr, const Tuple& tuple, const Schema& schema) {
    if (expr->expr_type == ExpressionType::COLUMN_REF) {
        const auto* col_expr = static_cast<const ColumnRefExpression*>(expr);
        int32_t col_idx = GetColIdx(schema, col_expr->col_name, col_expr->table_name);
        if (col_idx == -1) {
            throw std::runtime_error("Column not found in schema");
        }
        return tuple.GetValue(schema, col_idx);
    } else if (expr->expr_type == ExpressionType::CONSTANT) {
        const auto* const_expr = static_cast<const ConstantExpression*>(expr);
        if (const_expr->raw_value == "true" || const_expr->raw_value == "false") {
            Value val(TypeId::BOOLEAN);
            val.Set<int8_t>(const_expr->raw_value == "true" ? 1 : 0);
            return val;
        } else {
            Value val(TypeId::INTEGER);
            val.Set<int32_t>(std::stoi(const_expr->raw_value));
            return val;
        }
    }
    throw std::runtime_error("Invalid atom expression type");
}

bool EvaluateExpression(const Expression* expr, const Tuple& tuple, const Schema& schema) {
    if (!expr) return true;
    
    if (expr->expr_type == ExpressionType::LOGICAL) {
        const auto* log_expr = static_cast<const LogicalExpression*>(expr);
        bool left_res = EvaluateExpression(log_expr->left.get(), tuple, schema);
        
        if (log_expr->logic_type == LogicType::AND) {
            if (!left_res) return false;
            return EvaluateExpression(log_expr->right.get(), tuple, schema);
        } else {
            if (left_res) return true;
            return EvaluateExpression(log_expr->right.get(), tuple, schema);
        }
    } else if (expr->expr_type == ExpressionType::COMPARISON) {
        const auto* comp_expr = static_cast<const ComparisonExpression*>(expr);
        Value left_val = EvaluateAtom(comp_expr->left.get(), tuple, schema);
        Value right_val = EvaluateAtom(comp_expr->right.get(), tuple, schema);
        
        switch (comp_expr->comp_type) {
            case ComparisonType::EQ: return left_val.CompareEquals(right_val);
            case ComparisonType::NEQ: return left_val.CompareNotEqual(right_val);
            case ComparisonType::LT: return left_val.CompareLessThan(right_val);
            case ComparisonType::GT: return left_val.CompareGreaterThan(right_val);
            case ComparisonType::LTE: return left_val.CompareLessThanOrEqual(right_val);
            case ComparisonType::GTE: return left_val.CompareGreaterThanOrEqual(right_val);
        }
    }
    
    if (expr->expr_type == ExpressionType::COLUMN_REF || expr->expr_type == ExpressionType::CONSTANT) {
        Value val = EvaluateAtom(expr, tuple, schema);
        if (val.GetTypeId() == TypeId::BOOLEAN) {
            return val.Get<int8_t>() != 0;
        }
    }
    
    throw std::runtime_error("Invalid expression tree structure");
}
