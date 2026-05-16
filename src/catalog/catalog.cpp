#include "catalog/catalog.h"

TableInfo* Catalog::CreateTable(const std::string& table_name, const Schema& schema) {
    if (table_names_.find(table_name) != table_names_.end()) {
        throw std::runtime_error("Error: Table '" + table_name + "' already exists in catalog.");
    }

    table_oid_t oid = next_table_oid_++;
    auto table = std::make_unique<TableManager>();
    auto table_info = std::make_unique<TableInfo>(schema, table_name, std::move(table), oid);
    
    TableInfo* info_ptr = table_info.get();
    tables_[oid] = std::move(table_info);
    table_names_[table_name] = oid;
    
    return info_ptr;
}

TableInfo* Catalog::GetTable(const std::string& table_name) {
    auto it = table_names_.find(table_name);
    if (it == table_names_.end()) {
        throw std::runtime_error("Error: Table '" + table_name + "' not found in catalog.");
    }
    return GetTable(it->second);
}

TableInfo* Catalog::GetTable(table_oid_t table_oid) {
    auto it = tables_.find(table_oid);
    if (it == tables_.end()) {
        throw std::runtime_error("Error: Table with OID " + std::to_string(table_oid) + " not found in catalog.");
    }
    return it->second.get();
}

std::vector<TableInfo*> Catalog::GetAllTables() {
    std::vector<TableInfo*> result;
    for (auto& [oid, info] : tables_) {
        result.push_back(info.get());
    }
    return result;
}
