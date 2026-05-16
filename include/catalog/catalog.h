#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "manager/table_manager.h"
#include "catalog/schema.h"

using table_oid_t = uint32_t;

struct TableInfo {
    Schema schema_;
    std::string name_;
    std::unique_ptr<TableManager> table_;
    table_oid_t oid_;

    TableInfo(Schema schema, std::string name, std::unique_ptr<TableManager> table, table_oid_t oid)
        : schema_(std::move(schema)), name_(std::move(name)), table_(std::move(table)), oid_(oid) {}
};

class Catalog {
public:
    Catalog() = default;

    /**
     * @brief Creates a new table and registers it in the catalog.
     * Implement this to allocate a new TableManager, assign an OID,
     * store it in the tables_ and table_names_ maps, and return the TableInfo.
     */
    TableInfo* CreateTable(const std::string& table_name, const Schema& schema);

    /**
     * @brief Retrieves a table by name.
     * Implement this to look up the name in table_names_, then fetch from tables_.
     */
    TableInfo* GetTable(const std::string& table_name);

    /**
     * @brief Retrieves a table by OID.
     */
    TableInfo* GetTable(table_oid_t table_oid);

    /**
     * @brief Returns pointers to all registered tables.
     * Used by COMMIT to flush every table's buffer pool to disk.
     */
    std::vector<TableInfo*> GetAllTables();

    /**
     * @brief Serializes all table definitions (names, schemas, page IDs) to a binary file.
     * Called automatically on every CreateTable() and COMMIT.
     */
    void SaveCatalog(const std::string& path = "catalog.db");

    /**
     * @brief Deserializes table definitions from a binary file and reconstructs
     * the in-memory catalog. Called once on server startup.
     */
    void LoadCatalog(const std::string& path = "catalog.db");

private:
    std::unordered_map<table_oid_t, std::unique_ptr<TableInfo>> tables_;
    std::unordered_map<std::string, table_oid_t> table_names_;
    table_oid_t next_table_oid_{0};
};
