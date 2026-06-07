#pragma once
#include <vector>
#include <memory>
#include "query/expression.h"
#include "query/index_provider.h"

struct EquiCondition {
    std::string left_col;
    std::string right_col;
};

struct IndexCondition {
    std::string table;
    std::string col;
    std::string val;
};

struct BranchPlan {
    std::vector<IndexCondition> index_conditions;
    std::vector<EquiCondition> equi_conditions;
    std::unique_ptr<Expression> theta_filter; // anything not captured above
};

struct JoinPlan {
    std::vector<BranchPlan> branches;
};

class Optimizer {
public:
    static JoinPlan PlanJoin(const std::unique_ptr<Expression>& condition, const IndexProvider& idx_provider);
};
