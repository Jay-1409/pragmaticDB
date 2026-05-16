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
