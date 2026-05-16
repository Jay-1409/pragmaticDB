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

---

### `CREATE TABLE`

| Syntax | Status |
|---|---|
| `CREATE TABLE t (col TYPE);` | ✅ Implemented |
| `CREATE TABLE t (col1 TYPE, col2 TYPE, ...);` | ✅ Implemented |
| `CREATE TABLE IF NOT EXISTS t (...);` | ❌ Not implemented |

<details>
<summary>Example — single column</summary>

```sql
CREATE TABLE products (price INTEGER);
```
</details>

<details>
<summary>Example — multiple columns</summary>

```sql
CREATE TABLE users (id INTEGER, is_active BOOLEAN);
```
</details>

---

### `INSERT INTO`

| Syntax | Status |
|---|---|
| `INSERT INTO t VALUES (v1, v2, ...);` | ✅ Implemented |
| `INSERT INTO t (col1, col2) VALUES (v1, v2);` | ❌ Not implemented |
| `INSERT INTO t VALUES (...), (...), ...;` | ❌ Not implemented |

<details>
<summary>Example — insert a full row</summary>

```sql
INSERT INTO users VALUES (42, true);
INSERT INTO users VALUES (99, FALSE);
INSERT INTO users VALUES (7, True);
```

Values must match the column order from `CREATE TABLE`. Booleans are case-insensitive.
</details>

---

### `SELECT`

| Syntax | Status |
|---|---|
| `SELECT * FROM t;` | ✅ Implemented |
| `SELECT * FROM t WHERE col = val;` | 🚧 Under work |
| `SELECT col1, col2 FROM t;` | 🚧 Under work |
| `SELECT col1, col2 FROM t WHERE col = val;` | 🚧 Under work |
| `SELECT * FROM t ORDER BY col;` | 🚧 Under work |
| `SELECT * FROM t LIMIT n;` | 🚧 Under work |
| `SELECT * FROM t ORDER BY col LIMIT n;` | 🚧 Under work |
| `SELECT COUNT(*) FROM t;` | 🚧 Under work |
| `SELECT SUM(col) FROM t;` | 🚧 Under work |
| `SELECT AVG(col) FROM t;` | 🚧 Under work |
| `SELECT MIN(col) FROM t;` | 🚧 Under work |
| `SELECT MAX(col) FROM t;` | 🚧 Under work |
| `SELECT * FROM t1 JOIN t2 ON t1.col = t2.col;` | 🚧 Under work |

<details>
<summary>Example — select all rows</summary>

```sql
SELECT * FROM users;
```

Performs a full table scan. Output format: `col1 | col2 | ...` with a row count footer.
</details>

---

### `DELETE`

| Syntax | Status |
|---|---|
| `DELETE FROM t;` | ✅ Implemented |
| `DELETE FROM t WHERE col = val;` | ✅ Implemented |
| `DELETE FROM t WHERE col = val AND col2 = val2;` | ❌ Not implemented |
| `DELETE FROM t WHERE col > val;` | ❌ Not implemented |

<details>
<summary>Example — delete all rows</summary>

```sql
DELETE FROM users;
```
</details>

<details>
<summary>Example — delete by condition</summary>

```sql
DELETE FROM users WHERE id = 42;
```

Matches on exact equality for one column. Collected then deleted — does not modify during iteration.
</details>

---

### `UPDATE`

| Syntax | Status |
|---|---|
| `UPDATE t SET col = val;` | 🚧 Under work |
| `UPDATE t SET col = val WHERE col2 = val2;` | 🚧 Under work |
| `UPDATE t SET col1 = v1, col2 = v2 WHERE col3 = v3;` | 🚧 Under work |

---

### `DROP TABLE`

| Syntax | Status |
|---|---|
| `DROP TABLE t;` | 🚧 Under work |
| `DROP TABLE IF EXISTS t;` | 🚧 Under work |

---

### `SHOW TABLES`

| Syntax | Status |
|---|---|
| `SHOW TABLES;` | 🚧 Under work |

---

### `COMMIT`

| Syntax | Status |
|---|---|
| `COMMIT;` | ✅ Implemented |
| `COMMIT` (no semicolon) | ✅ Implemented |

<details>
<summary>Example</summary>

```sql
COMMIT;
```

- Flushes all dirty buffer-pool pages to `data/table_N.db`
- Updates `data/catalog.db` with the latest page map and schema
- All keywords are case-insensitive: `commit`, `COMMIT`, `Commit` all work
- Always run before stopping the server — unsaved inserts/deletes will be lost otherwise

</details>

---

### `exit` / `quit`

| Syntax | Status |
|---|---|
| `exit` | ✅ Implemented |
| `quit` | ✅ Implemented |

<details>
<summary>Example</summary>

```
exit
```

Closes the TCP connection. The server keeps running for new clients.  
**Does not flush data to disk** — run `COMMIT` first if you want your changes saved.

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