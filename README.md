![Build](https://img.shields.io/github/actions/workflow/status/jay-1409/pragmaticDB/ci.yml?branch=stable)
![License](https://img.shields.io/badge/license-MIT-green)
![Version](https://img.shields.io/badge/version-0.0.1-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B20-blue)
OPEN TO CONTIRBUTIONS
--
## pragmaticDB
An single binary sql engine for C++ applications that need structured, queryable storage.

## What is it?
PragmaticDB runs as a TCP server and speaks SQL. Point any client at it, send statements, get results back. No Postgres installation, no SQLite wrapper, no third-party runtime sitting between your application and its data. 

## Features
- **SQL over TCP** 
- **Persistent storage**  
- **Per-table isolation** 
- **Zero dependencies** 

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

## Some benchmarks
| Field | Value |
|---|---|
| Server | `127.0.0.1:8080` |
| N | `1000 inserts, 100 selects, 100 durable inserts` |

| Metric | Throughput | Latency |
|---|---:|---:|
| INSERT (in-memory, no COMMIT) | 15211 ops/sec | 0.066 ms/op |
| INSERT (durable, COMMIT each) | 5398 ops/sec | 0.185 ms/op |
| SELECT * (1000 rows/scan) | 112 ops/sec | 8.950 ms/op |

| Summary | Value |
|---|---:|
| COMMIT latency (1000 dirty pages) | 0.04 ms |
| In-memory vs durable speedup | 2.8x |

## Docs
- Example Queries [Examples](docs/examples.md)
- What are the limitations of this engine as of now? [Limitations](docs/limitations.md)
- **Sql reference sheet** [SQL reference](docs/sql-reference.md)
- How can i benchmark the database engine? [Benchmarking guide](docs/benchmarking.md)
- Whiteboard illustrations (excalidraw) [whiteboard](docs/whiteboard.excalidraw)

## License
MIT. See [LICENSE](LICENSE).
