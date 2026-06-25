## Insert
Then run SQL:
```sql
CREATE TABLE users (id INTEGER, is_active BOOLEAN);
INSERT INTO users VALUES (42, true);
INSERT INTO users VALUES (99, false);
SELECT * FROM users;
COMMIT;
```

Type `quit` or `exit` to disconnect.