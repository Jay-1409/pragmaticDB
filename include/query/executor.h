#pragma once

#include "catalog/catalog.h"
#include "../ds/statement.h"
#include "../ds/query_result.h"
#include "catalog/schema.h"
#include "query/optimizer.h"
#include "query/index_provider.h"
/**
 * @brief Executes a parsed Statement against the database Catalog.
 *
 * Takes ownership of a Statement* from the Parser and dispatches to the
 * correct handler, calling Catalog::CreateTable, TableManager::InsertTuple,
 * or scanning via TableIterator for SELECT.
 */
class Executor {
public:
    explicit Executor(Catalog& catalog, const IndexProvider& idx = kDefaultIndexProvider)
        : catalog_(catalog), index_provider_(idx) {}

    /**
     * @brief Execute the given statement and return a QueryResult.
     * Implement this to switch on statement.type and call the matching handler.
     */
    QueryResult Execute(const Statement& statement);

private:
    QueryResult ExecuteCreate(const CreateTableStatement& stmt);
    QueryResult ExecuteInsert(const InsertStatement& stmt);
    QueryResult ExecuteSelect(const SelectStatement& stmt);
    QueryResult ExecuteCommit();
    QueryResult ExecuteDelete(const DeleteStatement& stmt);

    QueryResult ExecuteJoin(const SelectStatement& stmt);
    std::vector<std::pair<RecordId, RecordId>> ExecuteBranch(
        const BranchPlan& branch,
        TableInfo* left_tbl, TableInfo* right_tbl,
        const Schema& left_schema, const Schema& right_schema,
        const Schema& merged_schema
    );

    Catalog& catalog_;
    const IndexProvider& index_provider_;
};
