#pragma once
#include "query/expression.h"
#include "type/tuple.h"
#include "catalog/schema.h"

// Evaluates any expression tree against a single (possibly merged) tuple.
// Returns true if the condition holds.
bool EvaluateExpression(
    const Expression* expr,
    const Tuple& tuple,
    const Schema& schema
);
