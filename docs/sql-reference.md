
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




## Case Sensitivity

| What | Case-sensitive? | Notes |
|---|---|---|
| SQL keywords (`CREATE`, `INSERT`, `SELECT`, `COMMIT`, ...) | No | `select`, `SELECT`, `Select` all work |
| Column types (`INTEGER`, `BOOLEAN`) | No | `integer`, `Boolean` all work |
| Boolean values (`true`, `false`) | No | `TRUE`, `False`, `FALSE` all work |
| Table names | **Yes** | `users` and `Users` are different tables |


