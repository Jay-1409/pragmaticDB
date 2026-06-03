#include <string>
#include "query/executor.h"
#include "type/tuple.h"
#include "type/value.h"
#include "factory/value_factory.h"
#include "query/expression_eval.h"
#include <unordered_set>
#include <unordered_map>

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
    if (!stmt.join_table_name.empty()) {
        return ExecuteJoin(stmt);
    }
    
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

static int GetSchemaColIdx(const Schema& schema, const std::string& table_name, const std::string& full_col_name) {
    for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
        std::string col_name = schema.GetColumn(i).GetName();
        if (col_name == full_col_name) return i;
        if (table_name + "." + col_name == full_col_name) return i;
        size_t dot_pos = full_col_name.find('.');
        if (dot_pos != std::string::npos && full_col_name.substr(dot_pos + 1) == col_name) return i;
    }
    return -1;
}

QueryResult Executor::ExecuteJoin(const SelectStatement& stmt) {
    try {
        TableInfo* left_tbl = catalog_.GetTable(stmt.table_name);
        TableInfo* right_tbl = catalog_.GetTable(stmt.join_table_name);
        if (!left_tbl || !right_tbl) throw std::runtime_error("Table not found for JOIN");

        Schema& left_schema = left_tbl->schema_;
        Schema& right_schema = right_tbl->schema_;
        Schema merged_schema = Schema::Merge(left_schema, left_tbl->name_, right_schema, right_tbl->name_);

        JoinPlan plan = Optimizer::PlanJoin(stmt.join_condition, index_provider_);

        std::unordered_set<std::string> seen;
        std::vector<std::pair<RecordId, RecordId>> unique_pairs;

        for (const auto& branch : plan.branches) {
            auto branch_pairs = ExecuteBranch(branch, left_tbl, right_tbl, left_schema, right_schema, merged_schema);

            for (const auto& pair : branch_pairs) {
                std::string key;
                key.append((char*)&pair.first.page_id, sizeof(page_id_t));
                key.append((char*)&pair.first.slot_id, sizeof(uint16_t));
                key.append((char*)&pair.second.page_id, sizeof(page_id_t));
                key.append((char*)&pair.second.slot_id, sizeof(uint16_t));

                if (seen.insert(key).second) {
                    unique_pairs.push_back(pair);
                }
            }
        }

        QueryResult result;
        result.success = true;
        for (const auto& pair : unique_pairs) {
            Tuple left_tuple = left_tbl->table_->GetTuple(pair.first, left_schema);
            Tuple right_tuple = right_tbl->table_->GetTuple(pair.second, right_schema);
            Tuple merged = Tuple::Merge(left_tuple, right_tuple);

            std::vector<std::string> row;
            for (uint32_t i = 0; i < merged_schema.GetColumnCount(); i++) {
                Value val = merged.GetValue(merged_schema, i);
                TypeId type = merged_schema.GetColumn(i).GetType();
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

std::vector<std::pair<RecordId, RecordId>> Executor::ExecuteBranch(
    const BranchPlan& branch,
    TableInfo* left_tbl, TableInfo* right_tbl,
    const Schema& left_schema, const Schema& right_schema,
    const Schema& merged_schema
) {
    std::vector<std::pair<RecordId, RecordId>> surviving_pairs;

    // Stage 1: Index Probe (Skipped for now)

    // Stage 2: Hash Join or Cross Product
    if (!branch.equi_conditions.empty()) {
        std::unordered_map<std::string, std::vector<std::pair<Tuple, RecordId>>> hash_map;

        // Build phase on right_tbl
        for (auto it = right_tbl->table_->Begin(right_schema); it != right_tbl->table_->End(right_schema); ++it) {
            std::string key;
            for (const auto& equi : branch.equi_conditions) {
                int col_idx = GetSchemaColIdx(right_schema, right_tbl->name_, equi.right_col);
                if (col_idx == -1) col_idx = GetSchemaColIdx(right_schema, right_tbl->name_, equi.left_col);
                if (col_idx != -1) {
                    Value val = (*it).GetValue(right_schema, col_idx);
                    key += ValueFactory::ToString(val, right_schema.GetColumn(col_idx).GetType()) + "|";
                }
            }
            if (!key.empty()) {
                hash_map[key].push_back({*it, it.GetRid()});
            }
        }

        // Probe phase on left_tbl
        for (auto it = left_tbl->table_->Begin(left_schema); it != left_tbl->table_->End(left_schema); ++it) {
            std::string key;
            for (const auto& equi : branch.equi_conditions) {
                int col_idx = GetSchemaColIdx(left_schema, left_tbl->name_, equi.left_col);
                if (col_idx == -1) col_idx = GetSchemaColIdx(left_schema, left_tbl->name_, equi.right_col);
                if (col_idx != -1) {
                    Value val = (*it).GetValue(left_schema, col_idx);
                    key += ValueFactory::ToString(val, left_schema.GetColumn(col_idx).GetType()) + "|";
                }
            }
            
            if (!key.empty() && hash_map.find(key) != hash_map.end()) {
                for (const auto& right_entry : hash_map[key]) {
                    Tuple merged = Tuple::Merge(*it, right_entry.first);
                    
                    // Stage 3: Theta Filter
                    if (branch.theta_filter) {
                        if (!EvaluateExpression(branch.theta_filter.get(), merged, merged_schema)) {
                            continue;
                        }
                    }
                    surviving_pairs.push_back({it.GetRid(), right_entry.second});
                }
            }
        }
    } else {
        // Cross product
        for (auto left_it = left_tbl->table_->Begin(left_schema); left_it != left_tbl->table_->End(left_schema); ++left_it) {
            for (auto right_it = right_tbl->table_->Begin(right_schema); right_it != right_tbl->table_->End(right_schema); ++right_it) {
                Tuple merged = Tuple::Merge(*left_it, *right_it);
                
                if (branch.theta_filter) {
                    if (!EvaluateExpression(branch.theta_filter.get(), merged, merged_schema)) {
                        continue;
                    }
                }
                surviving_pairs.push_back({left_it.GetRid(), right_it.GetRid()});
            }
        }
    }
    
    return surviving_pairs;
}
