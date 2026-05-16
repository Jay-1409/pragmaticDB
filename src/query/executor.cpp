#include <string>
#include "query/executor.h"
#include "type/tuple.h"
#include "type/value.h"
#include "factory/value_factory.h"

// Implement your Executor methods here!

QueryResult Executor::Execute(const Statement& statement) {
    switch (statement.type) {
        case StatementType::CREATE_TABLE:
            return ExecuteCreate(static_cast<const CreateTableStatement&>(statement));
        case StatementType::INSERT:
            return ExecuteInsert(static_cast<const InsertStatement&>(statement));
        case StatementType::SELECT:
            return ExecuteSelect(static_cast<const SelectStatement&>(statement));
        case StatementType::COMMIT:
            return ExecuteCommit();
        case StatementType::DELETE:
            return ExecuteDelete(static_cast<const DeleteStatement&>(statement));
    }
    return {false, "Unknown statement type", {}};
}

QueryResult Executor::ExecuteCommit() {
    auto all_tables = catalog_.GetAllTables();
    for (auto* table : all_tables) {
        table->table_->Flush();
    }
    // Update catalog.db with latest page IDs after flushing
    catalog_.SaveCatalog();
    return {true, "COMMIT: " + std::to_string(all_tables.size()) + " table(s) flushed to disk.", {}};
}

QueryResult Executor::ExecuteCreate(const CreateTableStatement& stmt) {
    try {
        catalog_.CreateTable(stmt.table_name, Schema(stmt.columns));
        return {true, "Table '" + stmt.table_name + "' created.", {}};
    } catch (const std::runtime_error& e) {
        return {false, e.what(), {}};
    }
}


QueryResult Executor::ExecuteInsert(const InsertStatement& stmt) {
    try {
        TableInfo* table = catalog_.GetTable(stmt.table_name);
        Schema& schema = table->schema_;
        std::vector<Value> values;
        for (int idx = 0; idx < (int)stmt.raw_values.size(); idx++) {
            TypeId type = schema.GetColumn(idx).GetType();
            values.push_back(ValueFactory::FromString(stmt.raw_values[idx], type));
        }
        Tuple tuple(values, schema);
        table->table_->InsertTuple(tuple);
        return {true, "1 row inserted.", {}};
    } catch (const std::runtime_error& e) {
        return {false, e.what(), {}};
    }
}

QueryResult Executor::ExecuteSelect(const SelectStatement& stmt) {
    try {
        TableInfo* tbl = catalog_.GetTable(stmt.table_name);
        Schema& schema = tbl->schema_;
        QueryResult result;
        result.success = true;
        for (auto it = tbl->table_->Begin(schema); it != tbl->table_->End(schema); ++it) {
            std::vector<std::string> row;
            for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
                Value val = (*it).GetValue(schema, i);
                TypeId type = schema.GetColumn(i).GetType();
                row.push_back(ValueFactory::ToString(val, type));
            }
            result.rows.push_back(row);
        }

        result.message = std::to_string(result.rows.size()) + " rows returned.";
        return result;

    } catch (const std::runtime_error& e) {
        return {false, e.what(), {}};
    }
}

QueryResult Executor::ExecuteDelete(const DeleteStatement& stmt) {
    try {
        TableInfo* tbl = catalog_.GetTable(stmt.table_name);
        Schema& schema = tbl->schema_;

        // Collect matching RecordIds first (can't delete while iterating)
        std::vector<RecordId> to_delete;

        // Find the WHERE column index (if a WHERE clause was given)
        int where_col_idx = -1;
        if (!stmt.where_column.empty()) {
            for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
                if (schema.GetColumn(i).GetName() == stmt.where_column) {
                    where_col_idx = static_cast<int>(i);
                    break;
                }
            }
            if (where_col_idx == -1) {
                return {false, "Error: Column '" + stmt.where_column + "' not found.", {}};
            }
        }

        for (auto it = tbl->table_->Begin(schema); it != tbl->table_->End(schema); ++it) {
            if (where_col_idx == -1) {
                // No WHERE clause: delete everything
                to_delete.push_back(it.GetRid());
            } else {
                // WHERE clause: check if this row matches
                Value val = (*it).GetValue(schema, where_col_idx);
                TypeId type = schema.GetColumn(where_col_idx).GetType();
                std::string row_val = ValueFactory::ToString(val, type);
                if (row_val == stmt.where_value) {
                    to_delete.push_back(it.GetRid());
                }
            }
        }

        for (const auto& rid : to_delete) {
            tbl->table_->DeleteTuple(rid);
        }

        return {true, std::to_string(to_delete.size()) + " row(s) deleted.", {}};

    } catch (const std::runtime_error& e) {
        return {false, e.what(), {}};
    }
}
