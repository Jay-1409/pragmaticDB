# Agent Instructions for pragmaticDB

These rules are **mandatory** for any AI coding assistant working on this project.
Violations of these rules are unacceptable under any circumstance.

---

## Rule 1: Zero Deletions

**Do NOT delete a single line of existing code.**

- No existing function, method, struct, class, enum, variable, comment, or include may be removed.
- No existing function signature may be changed in a way that breaks callers.
- If a function needs new parameters, use **default parameters** so all existing call sites compile unchanged.
- If a struct/class needs new fields, **append** them — never reorder, rename, or remove existing fields.
- New default values on appended fields must ensure the old behavior when the field is not explicitly set.

## Rule 2: Existing Features Must Not Change Behavior

**Every existing feature must continue to work exactly as it did before your changes.**

This includes but is not limited to:
- `CREATE TABLE`
- `INSERT INTO`
- `SELECT * FROM <table>;` (single-table select)
- `DELETE FROM <table>;` and `DELETE FROM <table> WHERE col = val;`
- `COMMIT;`
- `exit` / `quit`
- Persistence (catalog.db, table_N.db files)
- TCP server behavior

If you add a new code path (e.g., JOIN), it must be gated behind a condition check
so that existing queries never enter the new path. Example pattern:

```cpp
if (new_feature_is_active) {
    return NewFeaturePath();
}
// ... entire existing code below, untouched ...
```

## Rule 3: Additive-Only Architecture

All new features must be implemented as **additions**, not modifications:

- **New files** are always preferred over modifying existing files.
- When modifying an existing file is unavoidable, only **append** new code (new methods, new includes, new fields).
- The only acceptable in-body change to an existing function is inserting a short dispatch/guard at the **top** that routes to a new function, leaving the rest of the function body untouched.

## Rule 4: Test Preservation

- **Never modify or delete existing test logic.** Avoid modifying existing test files. If the project uses a central test runner/registry (e.g., `tests/test_main.cpp`, `include/tests.h`), appending a new test invocation or declaration there is allowed when necessary to wire new tests.
- All existing tests must continue to compile and pass after your changes.
- New tests go in **new test files** (e.g., `tests/test_join.cpp`).
- After making changes, verify that `make test` passes all existing tests.

## Rule 5: The Expression AST Is Join-Only

The Expression AST (`Expression`, `ComparisonExpression`, `LogicalExpression`, etc.)
and the expression evaluator (`EvaluateExpression`) are **exclusively** for JOIN condition evaluation.

- Do NOT refactor existing WHERE clause handling to use the AST.
- Do NOT refactor existing DELETE WHERE to use the AST.
- These existing features use their own simple string-comparison logic and must continue to do so.
- If a future feature needs expressions (e.g., SELECT WHERE with complex conditions),
  that is a separate, deliberate decision — not something to do as a side effect.

---

## Summary

| Action | Allowed? |
|---|---|
| Adding new files | ✅ Always |
| Appending new methods/fields to existing files | ✅ Yes |
| Adding a 3-line dispatch guard at the top of an existing function | ✅ Yes |
| Deleting any existing line of code | ❌ Never |
| Renaming any existing function, variable, or file | ❌ Never |
| Changing an existing function's behavior | ❌ Never |
| Modifying existing test files | ❌ Never |
| Using the Expression AST outside of JOIN | ❌ Never |
