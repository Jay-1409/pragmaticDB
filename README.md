![Build](https://img.shields.io/github/actions/workflow/status/jay-1409/pragmaticDB/ci.yml?branch=stable)
![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-0.0.1-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B20-blue)

## pragmaticDB
A tiny local SQL server for prototypes and learning.

## What is it?
pragmaticDB is a small C++20 database that runs as a local TCP server and accepts SQL statements line by line. It lets you create tables, insert rows, and read them back with `SELECT *`. Data is persisted to a single local file in the working directory, so results survive restarts. It is aimed at developers exploring database concepts or needing a minimal SQL surface for demos.

## Features
- **SQL over TCP:** Connect with any TCP client and send statements as plain text.
- **Simple schema:** Define tables with `INTEGER` and `BOOLEAN` columns.
- **Persistent storage:** Data is saved to disk between runs.
- **Local server:** One binary you can run on your machine without extra services.
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
```

Type `quit` or `exit` to disconnect.

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
No license file is present in this repository.