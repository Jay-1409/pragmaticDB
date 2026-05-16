/**
 * pragmaticDB Benchmark Client
 *
 * Connects to a running pragmaticDB server and measures:
 *   1. In-memory INSERT throughput (no COMMIT)
 *   2. COMMIT latency (flushing dirty pages to disk)
 *   3. SELECT * throughput (full table scan)
 *   4. Durable INSERT throughput (COMMIT after every insert)
 *
 * Usage:
 *   make bench
 *   ./bench [host] [port] [n_inserts]
 *   ./bench                  # defaults: 127.0.0.1 8080 1000
 */

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// ── TCP client ────────────────────────────────────────────────────────────────

class DBClient {
public:
    DBClient(const std::string& host, int port) {
        fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_ < 0) { std::cerr << "socket() failed\n"; exit(1); }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

        if (connect(fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "connect() failed — is the server running?\n"; exit(1);
        }
        drain(); // consume the welcome message + first prompt
    }

    ~DBClient() {
        send_line("exit");
        close(fd_);
    }

    // Send a SQL statement, return the server's response text.
    std::string query(const std::string& sql) {
        send_line(sql);
        return drain();
    }

private:
    int fd_;

    void send_line(const std::string& s) {
        std::string cmd = s + "\n";
        send(fd_, cmd.c_str(), cmd.size(), 0);
    }

    // Read until we see the "> " prompt the server sends after every response.
    std::string drain() {
        std::string buf;
        char tmp[4096];
        while (true) {
            int n = recv(fd_, tmp, sizeof(tmp) - 1, 0);
            if (n <= 0) break;
            tmp[n] = '\0';
            buf += tmp;
            if (buf.size() >= 2 && buf.substr(buf.size() - 2) == "> ") break;
        }
        return buf;
    }
};

// ── Helpers ───────────────────────────────────────────────────────────────────

using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

static void print_row(const std::string& label, double ms, int ops) {
    double ops_sec = ops / (ms / 1000.0);
    double lat_ms  = ms / ops;
    std::cout << std::left  << std::setw(36) << label
              << std::right << std::setw(10) << std::fixed << std::setprecision(0) << ops_sec << " ops/sec"
              << std::setw(12) << std::setprecision(3) << lat_ms << " ms/op\n";
}

static void separator() { std::cout << std::string(60, '-') << "\n"; }

// ── Main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
    int  port        = (argc > 2) ? std::stoi(argv[2]) : 8080;
    int  N           = (argc > 3) ? std::stoi(argv[3]) : 1000; // insert count
    int  S           = std::max(10, N / 10);  // select count (fewer — each scans N rows)
    int  D           = std::max(10, N / 10);  // durable insert count

    std::cout << "\n=== pragmaticDB Benchmark ===\n";
    std::cout << "Server : " << host << ":" << port << "\n";
    std::cout << "N      : " << N << " inserts, " << S << " selects, " << D << " durable inserts\n\n";

    DBClient db(host, port);

    // ── Setup: create or clean the bench table ─────────────────────────────
    db.query("CREATE TABLE bench (id INTEGER, val INTEGER);"); // ok if it already exists (error ignored)
    db.query("DELETE FROM bench;");
    db.query("COMMIT;");

    // ── Phase 1: in-memory INSERT throughput ──────────────────────────────
    auto t0 = Clock::now();
    for (int i = 0; i < N; i++) {
        db.query("INSERT INTO bench VALUES (" + std::to_string(i) + ", " + std::to_string(i * 2) + ");");
    }
    double insert_ms = Ms(Clock::now() - t0).count();

    // ── Phase 2: COMMIT latency ────────────────────────────────────────────
    auto tc0 = Clock::now();
    db.query("COMMIT;");
    double commit_ms = Ms(Clock::now() - tc0).count();

    // ── Phase 3: SELECT * throughput (full scan, N rows per query) ─────────
    auto ts0 = Clock::now();
    for (int i = 0; i < S; i++) {
        db.query("SELECT * FROM bench;");
    }
    double select_ms = Ms(Clock::now() - ts0).count();

    // ── Phase 4: durable INSERT (COMMIT after every insert) ────────────────
    db.query("DELETE FROM bench;");
    db.query("COMMIT;");

    auto td0 = Clock::now();
    for (int i = 0; i < D; i++) {
        db.query("INSERT INTO bench VALUES (" + std::to_string(i) + ", " + std::to_string(i) + ");");
        db.query("COMMIT;");
    }
    double durable_ms = Ms(Clock::now() - td0).count();

    // ── Results ────────────────────────────────────────────────────────────
    separator();
    std::cout << std::left << std::setw(36) << "Metric"
              << std::right << std::setw(18) << "Throughput"
              << std::setw(16) << "Latency\n";
    separator();

    print_row("INSERT (in-memory, no COMMIT)",   insert_ms,  N);
    print_row("INSERT (durable, COMMIT each)",   durable_ms, D);
    print_row("SELECT * (" + std::to_string(N) + " rows/scan)", select_ms, S);

    separator();
    std::cout << "COMMIT latency (" << N << " dirty pages): "
              << std::fixed << std::setprecision(2) << commit_ms << " ms\n";
    std::cout << "In-memory vs durable speedup: "
              << std::setprecision(1) << (durable_ms / D) / (insert_ms / N) << "x\n";
    separator();

    return 0;
}
