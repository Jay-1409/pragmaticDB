#pragma once

#include "catalog/catalog.h"
#include "../ds/statement.h"
#include "../ds/query_result.h"
#include "catalog/schema.h"
#include "kv_store/kv_store.h"
/**
 * @brief Executes a parsed Statement against the database Catalog.
 *
 * Takes ownership of a Statement* from the Parser and dispatches to the
 * correct handler, calling Catalog::CreateTable, TableManager::InsertTuple,
 * or scanning via TableIterator for SELECT.
 */
class Executor {
public:
    explicit Executor(Catalog& catalog) : catalog_(catalog) {
        kv_store_.Load(); // Load KV store state from disk on startup
    }

    /**
     * @brief Execute the given statement and return a QueryResult.
     * Implement this to switch on statement.type and call the matching handler.
     */
    QueryResult Execute(const Statement& statement);

    KVStore& GetKVStore() { return kv_store_; }

private:
    QueryResult ExecuteCreate(const CreateTableStatement& stmt);
    QueryResult ExecuteInsert(const InsertStatement& stmt);
    QueryResult ExecuteSelect(const SelectStatement& stmt);
    QueryResult ExecuteCommit();
    QueryResult ExecuteDelete(const DeleteStatement& stmt);

    // KV handlers
    QueryResult ExecuteKVPut(const KVPutStatement& stmt);
    QueryResult ExecuteKVGet(const KVGetStatement& stmt);
    QueryResult ExecuteKVDelete(const KVDeleteStatement& stmt);
    QueryResult ExecuteKVExists(const KVExistsStatement& stmt);

    Catalog& catalog_;
    KVStore  kv_store_;
};
