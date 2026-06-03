#include "query/optimizer.h"

static void CollectORBranches(const Expression* expr, std::vector<const Expression*>& branches) {
    if (!expr) return;
    if (expr->expr_type == ExpressionType::LOGICAL) {
        const auto* log = static_cast<const LogicalExpression*>(expr);
        if (log->logic_type == LogicType::OR) {
            CollectORBranches(log->left.get(), branches);
            CollectORBranches(log->right.get(), branches);
            return;
        }
    }
    branches.push_back(expr);
}

static void CollectANDConditions(const Expression* expr, std::vector<const Expression*>& conds) {
    if (!expr) return;
    if (expr->expr_type == ExpressionType::LOGICAL) {
        const auto* log = static_cast<const LogicalExpression*>(expr);
        if (log->logic_type == LogicType::AND) {
            CollectANDConditions(log->left.get(), conds);
            CollectANDConditions(log->right.get(), conds);
            return;
        }
    }
    conds.push_back(expr);
}

static std::unique_ptr<Expression> CopyExpression(const Expression* expr) {
    if (!expr) return nullptr;
    if (expr->expr_type == ExpressionType::COLUMN_REF) {
        const auto* col = static_cast<const ColumnRefExpression*>(expr);
        return std::make_unique<ColumnRefExpression>(col->table_name, col->col_name);
    } else if (expr->expr_type == ExpressionType::CONSTANT) {
        const auto* con = static_cast<const ConstantExpression*>(expr);
        return std::make_unique<ConstantExpression>(con->raw_value);
    } else if (expr->expr_type == ExpressionType::COMPARISON) {
        const auto* comp = static_cast<const ComparisonExpression*>(expr);
        return std::make_unique<ComparisonExpression>(
            comp->comp_type, CopyExpression(comp->left.get()), CopyExpression(comp->right.get()));
    } else if (expr->expr_type == ExpressionType::LOGICAL) {
        const auto* log = static_cast<const LogicalExpression*>(expr);
        return std::make_unique<LogicalExpression>(
            log->logic_type, CopyExpression(log->left.get()), CopyExpression(log->right.get()));
    }
    return nullptr;
}

static std::unique_ptr<Expression> CombineAND(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) {
    if (!left) return right;
    if (!right) return left;
    return std::make_unique<LogicalExpression>(LogicType::AND, std::move(left), std::move(right));
}

JoinPlan Optimizer::PlanJoin(const std::unique_ptr<Expression>& condition, const IndexProvider& idx_provider) {
    JoinPlan plan;
    if (!condition) {
        BranchPlan empty_branch;
        plan.branches.push_back(std::move(empty_branch));
        return plan;
    }
    
    std::vector<const Expression*> or_branches;
    CollectORBranches(condition.get(), or_branches);
    
    for (const Expression* branch_expr : or_branches) {
        BranchPlan b_plan;
        std::vector<const Expression*> and_conds;
        CollectANDConditions(branch_expr, and_conds);
        
        for (const Expression* cond : and_conds) {
            bool handled = false;
            if (cond->expr_type == ExpressionType::COMPARISON) {
                const auto* comp = static_cast<const ComparisonExpression*>(cond);
                if (comp->comp_type == ComparisonType::EQ) {
                    if (comp->left->expr_type == ExpressionType::COLUMN_REF && comp->right->expr_type == ExpressionType::COLUMN_REF) {
                        const auto* l_col = static_cast<const ColumnRefExpression*>(comp->left.get());
                        const auto* r_col = static_cast<const ColumnRefExpression*>(comp->right.get());
                        
                        EquiCondition equi;
                        equi.left_col = (l_col->table_name.empty() ? l_col->col_name : l_col->table_name + "." + l_col->col_name);
                        equi.right_col = (r_col->table_name.empty() ? r_col->col_name : r_col->table_name + "." + r_col->col_name);
                        
                        b_plan.equi_conditions.push_back(equi);
                        handled = true;
                    }
                }
            }
            if (!handled) {
                b_plan.theta_filter = CombineAND(std::move(b_plan.theta_filter), CopyExpression(cond));
            }
        }
        
        plan.branches.push_back(std::move(b_plan));
    }
    
    return plan;
}
