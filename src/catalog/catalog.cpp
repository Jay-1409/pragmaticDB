#include "catalog/catalog.h"
#include <fstream>
#include <stdexcept>

// ── Helpers ────────────────────────────────────────────────────────────────────

static void write_string(std::ofstream& out, const std::string& s) {
    uint32_t len = s.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    out.write(s.data(), len);
}

static std::string read_string(std::ifstream& in) {
    uint32_t len = 0;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string s(len, '\0');
    in.read(s.data(), len);
    return s;
}

// ── Core CRUD ─────────────────────────────────────────────────────────────────

TableInfo* Catalog::CreateTable(const std::string& table_name, const Schema& schema) {
    if (table_names_.find(table_name) != table_names_.end()) {
        throw std::runtime_error("Error: Table '" + table_name + "' already exists in catalog.");
    }

    table_oid_t oid = next_table_oid_++;
    std::string filename = "table_" + std::to_string(oid) + ".db";
    auto table = std::make_unique<TableManager>(filename);
    auto table_info = std::make_unique<TableInfo>(schema, table_name, std::move(table), oid);
    
    TableInfo* info_ptr = table_info.get();
    tables_[oid] = std::move(table_info);
    table_names_[table_name] = oid;

    // Auto-persist catalog on every schema change
    SaveCatalog();

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

// ── Persistence ───────────────────────────────────────────────────────────────
//
// Binary layout of catalog.db:
//   [next_table_oid: uint32_t]
//   [num_tables:     uint32_t]
//   For each table:
//     [oid:          uint32_t]
//     [name:         uint32_t len + char[len]]
//     [num_columns:  uint32_t]
//     For each column:
//       [col_name:   uint32_t len + char[len]]
//       [type_id:    uint32_t]
//     [num_pages:    uint32_t]
//     For each page:
//       [page_id:    int32_t]

void Catalog::SaveCatalog(const std::string& path) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return;

    // Header
    out.write(reinterpret_cast<const char*>(&next_table_oid_), sizeof(next_table_oid_));
    uint32_t num_tables = tables_.size();
    out.write(reinterpret_cast<const char*>(&num_tables), sizeof(num_tables));

    for (auto& [oid, info] : tables_) {
        // OID + name
        out.write(reinterpret_cast<const char*>(&oid), sizeof(oid));
        write_string(out, info->name_);

        // Columns
        const auto& columns = info->schema_.GetColumns();
        uint32_t num_cols = columns.size();
        out.write(reinterpret_cast<const char*>(&num_cols), sizeof(num_cols));
        for (const auto& col : columns) {
            write_string(out, col.GetName());
            uint32_t type = static_cast<uint32_t>(col.GetType());
            out.write(reinterpret_cast<const char*>(&type), sizeof(type));
        }

        // Page IDs (so we know which disk pages belong to this table)
        const auto& page_ids = info->table_->GetPageIds();
        uint32_t num_pages = page_ids.size();
        out.write(reinterpret_cast<const char*>(&num_pages), sizeof(num_pages));
        for (int32_t pid : page_ids) {
            out.write(reinterpret_cast<const char*>(&pid), sizeof(pid));
        }

        // next_page_id so on reload we don't re-allocate existing pages
        page_id_t next_pid = info->table_->GetNextPageId();
        out.write(reinterpret_cast<const char*>(&next_pid), sizeof(next_pid));
    }
}

void Catalog::LoadCatalog(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return; // No catalog file yet — fresh start

    // Header
    in.read(reinterpret_cast<char*>(&next_table_oid_), sizeof(next_table_oid_));
    uint32_t num_tables = 0;
    in.read(reinterpret_cast<char*>(&num_tables), sizeof(num_tables));

    for (uint32_t t = 0; t < num_tables; ++t) {
        // OID + name
        table_oid_t oid = 0;
        in.read(reinterpret_cast<char*>(&oid), sizeof(oid));
        std::string name = read_string(in);

        // Columns → rebuild Schema
        uint32_t num_cols = 0;
        in.read(reinterpret_cast<char*>(&num_cols), sizeof(num_cols));
        std::vector<Column> columns;
        for (uint32_t c = 0; c < num_cols; ++c) {
            std::string col_name = read_string(in);
            uint32_t type_raw = 0;
            in.read(reinterpret_cast<char*>(&type_raw), sizeof(type_raw));
            columns.emplace_back(col_name, static_cast<TypeId>(type_raw));
        }
        Schema schema(columns);

        // Page IDs → restore which disk pages belong to this table
        uint32_t num_pages = 0;
        in.read(reinterpret_cast<char*>(&num_pages), sizeof(num_pages));
        std::vector<page_id_t> page_ids;
        for (uint32_t p = 0; p < num_pages; ++p) {
            int32_t pid = 0;
            in.read(reinterpret_cast<char*>(&pid), sizeof(pid));
            page_ids.push_back(pid);
        }

        // next_page_id → prevent re-allocating existing pages on new inserts
        page_id_t next_pid = 0;
        in.read(reinterpret_cast<char*>(&next_pid), sizeof(next_pid));

        // Reconstruct TableInfo using the OID-based filename
        std::string filename = "table_" + std::to_string(oid) + ".db";
        auto table = std::make_unique<TableManager>(filename);
        table->SetPageIds(page_ids);
        table->SetNextPageId(next_pid);
        auto info = std::make_unique<TableInfo>(schema, name, std::move(table), oid);

        tables_[oid] = std::move(info);
        table_names_[name] = oid;
    }
}
