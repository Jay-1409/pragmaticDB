#pragma once

#include <cstdint>
#include "query/parser.h"
#include "query/executor.h"

class TcpServer {
public:
    TcpServer(Executor& executor, Parser& parser);
    ~TcpServer();

    /**
     * @brief Start the server listening on the given port.
     * Enters an infinite loop accepting connections.
     */
    void Start(uint16_t port);

private:
    /**
     * @brief Handles a single connected client.
     */
    void HandleClient(int client_socket);

    /**
     * @brief Format the QueryResult into a string for the client.
     */
    std::string FormatResult(const QueryResult& result);

    Executor& executor_;
    Parser& parser_;
    int server_socket_;
};
