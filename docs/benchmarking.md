# Run benchmarking yourself
---

## Guide

pragmaticDB includes a built-in benchmark client (`tools/bench.cpp`) that connects to a live server and measures insert throughput, full-table scan speed, and commit (fsync) latency. This is particularly useful for measuring the real-world impact of the LRU-K Buffer Pool vs direct durable writes.

First, ensure the pragmaticDB server is running in a terminal:
```bash
make run
```

In a second terminal, build and run the benchmark suite:
```bash
make bench
./bench [host] [port] [number_of_inserts]

# Defaults to:
./bench 127.0.0.1 8080 1000
```

## Metrices 

| Metric | What it shows |
|---|---|
| **In-memory INSERT** | Raw throughput of the buffer pool + slotted page writes. High ops/sec because it writes to RAM. |
| **Durable INSERT** | Cost of `COMMIT` (fsync) after *every single* write. Shows worst-case persistence overhead. |
| **Speedup ratio** | The ratio of in-memory speed vs durable speed. Proves the buffer pool is doing real work (typically 10–50x speedup). |
| **SELECT * scan** | Full table scan throughput across all inserted rows. |
| **COMMIT latency** | Time taken to flush *N* dirty pages from RAM to disk atomically. |

*(benchmarking results, macbook air m4)*  
<img width="656" height="251" alt="image" src="https://github.com/user-attachments/assets/7529c3e4-fa13-4cf0-8f41-9ccb801acf83" />

## Example
*(benchmarking results, macbook air m4)*  
<img width="656" height="251" alt="image" src="https://github.com/user-attachments/assets/7529c3e4-fa13-4cf0-8f41-9ccb801acf83" />