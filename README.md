![Build](https://img.shields.io/github/actions/workflow/status/jay-1409/pragmaticDB/ci.yml?branch=stable)
![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-0.0.1-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B20-blue)

## pragmaticDB
An embedded SQL server for C++ applications that need structured, queryable storage.

## What is it?
pragmaticDB is a C++20 database that runs as a TCP server and accepts SQL statements line by line. It lets you create tables, insert rows, and read them back with `SELECT *`. Both table schemas and row data survive server restarts — type `COMMIT;` to flush everything to disk and it will all be there when you start the server again. It is for C++ developers who want SQL queries and persistence without running an external database service.

## Features
- **SQL over TCP:** Connect with any TCP client and send statements as plain text.
- **Simple schema:** Define tables with `INTEGER` and `BOOLEAN` columns.
- **Full persistence:** `COMMIT;` saves both schemas and row data to disk. Everything survives a server restart.
- **Isolated table storage:** Each table lives in its own file inside `data/`, preventing any cross-table data corruption.
- **Single binary:** Run the server without extra services or dependencies.
- **Readable output:** Results are returned as line-oriented text.
- **Quick build:** Build and run with `make`.

## Installation / Build
Requirements: a C++20-capable compiler and `make`.

```bash
make app
make test
```

## Quick Start
Start the server and connect with a TCP client.

```bash
make run
```

In another terminal:
```bash
nc localhost 8080
```

Then run SQL:
```sql
CREATE TABLE users (id INTEGER, is_active BOOLEAN);
INSERT INTO users VALUES (42, true);
INSERT INTO users VALUES (99, false);
SELECT * FROM users;
COMMIT;
```

Type `quit` or `exit` to disconnect.

## SQL Reference

### DDL — Data Definition Language

| Command | Status | Description |
|---|---|---|
| `CREATE TABLE` | ✅ Available | Create a new table with typed columns |
| `DROP TABLE` | 🚧 Under work | Remove a table and all its data |
| `ALTER TABLE` | ❌ Not planned | Modify an existing table's schema |

### DML — Data Manipulation Language

| Command | Status | Description |
|---|---|---|
| `INSERT INTO` | ✅ Available | Insert a row of values into a table |
| `SELECT * FROM` | ✅ Available | Read all rows from a table (full scan) |
| `DELETE FROM` | ✅ Available | Delete rows, optionally filtered by `WHERE` |
| `UPDATE ... SET` | 🚧 Under work | Modify existing row values |
| `SELECT * FROM ... WHERE` | 🚧 Under work | Filter rows by column equality |
| `SELECT col, col FROM` | 🚧 Under work | Project specific columns instead of `*` |
| `ORDER BY` | 🚧 Under work | Sort result rows |
| `LIMIT` | 🚧 Under work | Cap the number of rows returned |
| `JOIN` | 🚧 Under work | Combine rows from multiple tables |
| `GROUP BY` / `HAVING` | 🚧 Under work | Aggregate rows by grouping |

### Aggregates

| Function | Status | Description |
|---|---|---|
| `COUNT(*)` | 🚧 Under work | Count number of rows |
| `SUM(col)` | 🚧 Under work | Sum of values in a column |
| `AVG(col)` | 🚧 Under work | Average of values in a column |
| `MIN(col)` / `MAX(col)` | 🚧 Under work | Minimum / maximum value in a column |

### Transactions & Control

| Command | Status | Description |
|---|---|---|
| `COMMIT` | ✅ Available | Flush all pending data and catalog to disk |
| `BEGIN` | 🚧 Under work | Start an explicit transaction |
| `ROLLBACK` | 🚧 Under work | Undo changes since last `BEGIN` |

### Catalog & Utilities

| Command | Status | Description |
|---|---|---|
| `SHOW TABLES` | 🚧 Under work | List all tables in the catalog |
| `exit` / `quit` | ✅ Available | Disconnect the current client session |

---

<details>
<summary><strong>CREATE TABLE</strong> — define a new table</summary>

```sql
CREATE TABLE users (id INTEGER, is_active BOOLEAN);
CREATE TABLE products (price INTEGER);
```

- Column types supported: `INTEGER` (4 bytes), `BOOLEAN` (1 byte)
- Type names are case-insensitive: `integer`, `INTEGER`, `Integer` all work
- The schema is written to `data/catalog.db` immediately on creation

</details>

<details>
<summary><strong>INSERT INTO</strong> — add a row</summary>

```sql
INSERT INTO users VALUES (42, true);
INSERT INTO users VALUES (99, FALSE);
INSERT INTO users VALUES (7, True);
```

- Values must match the column order defined in `CREATE TABLE`
- Boolean values are case-insensitive: `true`, `TRUE`, `True`, `false`, `FALSE`, `False`
- Inserted rows live in the buffer pool RAM until you `COMMIT`

</details>

<details>
<summary><strong>SELECT * FROM</strong> — read all rows</summary>

```sql
SELECT * FROM users;
SELECT * FROM products;
```

- Returns every column for every row in the table
- Performs a full table scan (no indexes)
- Output format: `col1 | col2 | col3` with a row count at the end

</details>

<details>
<summary><strong>DELETE FROM</strong> — remove rows</summary>

```sql
-- Delete a specific row
DELETE FROM users WHERE id = 42;

-- Delete all rows in the table
DELETE FROM users;
```

- `WHERE` supports equality checks (`=`) on a single column
- Without `WHERE`, all rows are deleted
- Deletions are in-memory until you `COMMIT`

</details>

<details>
<summary><strong>COMMIT</strong> — persist everything to disk</summary>

```sql
COMMIT;
```

- Flushes all dirty buffer-pool pages to their `data/table_N.db` file
- Updates `data/catalog.db` with the latest page locations and schema info
- All SQL keywords are **case-insensitive** — `commit`, `COMMIT`, `Commit` all work
- Always run before stopping the server, otherwise unsaved inserts/deletes are lost

</details>

<details>
<summary><strong>exit / quit</strong> — disconnect from the server</summary>

```
exit
quit
```

- Closes the current TCP client connection
- The server stays running and accepts new connections
- Does **not** flush data to disk — run `COMMIT` before disconnecting if you want your data saved

</details>



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

## Case Sensitivity

| What | Case-sensitive? | Notes |
|---|---|---|
| SQL keywords (`CREATE`, `INSERT`, `SELECT`, `COMMIT`, ...) | No | `select`, `SELECT`, `Select` all work |
| Column types (`INTEGER`, `BOOLEAN`) | No | `integer`, `Boolean` all work |
| Boolean values (`true`, `false`) | No | `TRUE`, `False`, `FALSE` all work |
| Table names | **Yes** | `users` and `Users` are different tables |

## Configuration
No user-configurable options are exposed yet.

## Limitations
- SQL support is limited to `CREATE TABLE`, `INSERT`, `SELECT *`, and `COMMIT`.
- Only `INTEGER` and `BOOLEAN` column types are supported.
- No `WHERE`, `UPDATE`, `DELETE`, `JOIN`, `ORDER BY`, or aggregation features.
- No automatic transactions or concurrency control.
- No indexes — `SELECT *` always performs a full table scan.
- Server listens on port 8080 and accepts one client at a time.
- No authentication or TLS.

## Contributing
```bash
make app
make test
make clean
```

Open a PR with a clear description of the change and how you tested it.

## License
MIT. See [LICENSE](LICENSE).