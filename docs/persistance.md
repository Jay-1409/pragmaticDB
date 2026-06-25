
## Persistence
All database files are stored in the `data/` directory inside your working directory.

```
data/
├── catalog.db      ← table registry: names, schemas, page ownership, OIDs
├── table_0.db      ← raw page data for the first table created
├── table_1.db      ← raw page data for the second table created
└── ...             ← one file per table
```

**How it works:**
- `CREATE TABLE` immediately writes the schema to `data/catalog.db`.
- `INSERT` rows live in RAM (buffer pool) until you `COMMIT`.
- `COMMIT` flushes all dirty pages to the corresponding `table_N.db` file and updates `catalog.db` with the latest page locations.
- On the next `make run`, the server reads `catalog.db` and all your tables are already registered — no need to recreate them.

**Starting fresh:** Delete the `data/` directory to wipe all tables and data.
```bash
rm -rf data/
```