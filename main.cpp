#include <iostream>
#include "catalog/catalog.h"
#include "query/parser.h"
#include "query/executor.h"
#include "network/tcp_server.h"

int main() {
    std::cout << "Starting pragmaticDB Server...\n";

    // Initialize core components
    Catalog catalog;
    Executor executor(catalog);
    Parser parser;

    // Start the network layer
    TcpServer server(executor, parser);
    
    // This will block infinitely and serve clients
    server.Start(8080);

    return 0;
}
