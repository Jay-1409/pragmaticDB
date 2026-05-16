![Build](https://img.shields.io/badge/build-unknown-lightgrey)
![License](https://img.shields.io/badge/license-unknown-lightgrey)
![Version](https://img.shields.io/badge/version-unknown-lightgrey)
![Language](https://img.shields.io/badge/language-C%2B%2B20-blue)

## StableDB
A minimal embedded SQL database for small C++ projects and demos.

## What is it?
StableDB is a small C++20 database that lets you define tables, insert rows, and read them back with SQL. It runs in-process and stores data in a single local file in the working directory. The SQL surface is intentionally small so behavior is easy to understand and test. It is aimed at developers exploring database ideas or needing a lightweight SQL layer for prototypes.

## Features
- **Simple SQL:** Supports `CREATE TABLE`, `INSERT`, and `SELECT *`.
- **Typed columns:** `INTEGER` and `BOOLEAN` columns are supported.
- **In-process usage:** No server or daemon to run.
- **Persistent data:** Rows are stored on disk between runs.
- **Table reads:** `SELECT *` returns all rows in a table.
- **Single-command build:** Build and run tests with `make`.

## Installation / Build
Requirements: a C++20-capable compiler and `make`.

```bash
make app
make test
```

## Quick Start
Build and run the test runner, which exercises the SQL below (see [tests/test_query_engine.cpp](tests/test_query_engine.cpp)).

```bash
make test
```

```sql
CREATE TABLE users (id INTEGER, is_active BOOLEAN);
INSERT INTO users VALUES (42, true);
INSERT INTO users VALUES (99, false);
SELECT * FROM users;
```

## Usage
Create a table with typed columns.
```sql
CREATE TABLE users (id INTEGER, is_active BOOLEAN);
```

Insert rows into a table.
```sql
INSERT INTO users VALUES (1, true);
INSERT INTO users VALUES (2, false);
```

Read all rows from a table.
```sql
SELECT * FROM users;
```

## Configuration
No user-configurable options are exposed yet.

## Limitations
- SQL support is limited to `CREATE TABLE`, `INSERT`, and `SELECT *`.
- Only `INTEGER` and `BOOLEAN` column types are supported.
- No `WHERE`, `UPDATE`, `DELETE`, `JOIN`, `ORDER BY`, or aggregation features.
- No transactions, concurrency control, or indexes.
- No interactive CLI or network server is included.

## Contributing
```bash
make app
make test
make clean
```

Open a PR with a clear description of the change and how you tested it.

## License
No license file is present in this repository.