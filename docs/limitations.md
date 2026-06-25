# Current limitations
---

- SQL support is limited to `CREATE TABLE`, `INSERT`, `SELECT *`, and `COMMIT`.
- Only `INTEGER` and `BOOLEAN` column types are supported.
- No `WHERE`, `UPDATE`, `DELETE`, `JOIN`, `ORDER BY`, or aggregation features.
- No automatic transactions or concurrency control.
- No indexes — `SELECT *` always performs a full table scan.
- Server listens on port 8080 and accepts one client at a time.
- No authentication or TLS.