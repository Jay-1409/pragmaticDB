#pragma once

#include "catalog/catalog.h"
#include "../ds/statement.h"
#include "../ds/query_result.h"
#include "catalog/schema.h"
/**
 * @brief Executes a parsed Statement against the database Catalog.
 *
 * Takes ownership of a Statement* from the Parser and dispatches to the
 * correct handler, calling Catalog::CreateTable, TableManager::InsertTuple,
 * or scanning via TableIterator for SELECT.
 */
class Executor {
public:
    explicit Executor(Catalog& catalog) : catalog_(catalog) {}

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

    Catalog& catalog_;
};
