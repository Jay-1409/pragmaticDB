#include "network/tcp_server.h"
#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

TcpServer::TcpServer(Executor& executor, Parser& parser) 
    : executor_(executor), parser_(parser), server_socket_(-1) {}

TcpServer::~TcpServer() {
    if (server_socket_ != -1) {
        close(server_socket_);
    }
}

void TcpServer::Start(uint16_t port) {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Failed to create socket.\n";
        return;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_socket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed. Port might be in use.\n";
        return;
    }

    if (listen(server_socket_, 5) < 0) {
        std::cerr << "Listen failed.\n";
        return;
    }

    std::cout << "pragmaticDB Server listening on port " << port << "...\n";

    while (true) {
        int client_socket = accept(server_socket_, nullptr, nullptr);
        if (client_socket < 0) {
            std::cerr << "Accept failed.\n";
            continue;
        }

        std::cout << "Client connected.\n";
        HandleClient(client_socket);
        close(client_socket);
        std::cout << "Client disconnected.\n";
    }
}

void TcpServer::HandleClient(int client_socket) {
    char buffer[1024];

    // Send a welcome message
    std::string welcome = "Welcome to pragmaticDB! Type your SQL queries below.\n> ";
    send(client_socket, welcome.c_str(), welcome.length(), 0);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(client_socket, buffer, sizeof(buffer) - 1);
        
        if (valread <= 0) {
            break; // Client disconnected or error
        }

        std::string input(buffer);
        
        // Remove trailing newlines/carriage returns
        while (!input.empty() && (input.back() == '\n' || input.back() == '\r')) {
            input.pop_back();
        }

        if (input.empty()) {
            std::string prompt = "> ";
            send(client_socket, prompt.c_str(), prompt.length(), 0);
            continue;
        }

        if (input == "exit" || input == "quit") {
            std::string msg = "Bye!\n";
            send(client_socket, msg.c_str(), msg.length(), 0);
            break;
        }

        std::cout << "Received query: " << input << "\n";

        auto stmt = parser_.Parse(input);
        std::string response;

        if (!stmt) {
            response = "Error: Invalid or unsupported SQL statement.\n";
        } else {
            QueryResult result = executor_.Execute(*stmt);
            response = FormatResult(result);
        }

        std::string prompt = "\n> ";
        send(client_socket, response.c_str(), response.length(), 0);
        send(client_socket, prompt.c_str(), prompt.length(), 0);
    }
}

std::string TcpServer::FormatResult(const QueryResult& result) {
    std::ostringstream oss;
    
    if (!result.success) {
        oss << "Error: " << result.message << "\n";
        return oss.str();
    }

    if (!result.rows.empty()) {
        for (const auto& row : result.rows) {
            for (size_t i = 0; i < row.size(); ++i) {
                oss << row[i];
                if (i < row.size() - 1) oss << " | ";
            }
            oss << "\n";
        }
    }
    
    oss << result.message << "\n";
    return oss.str();
}

/** this is the actual interface that the user /client talks with in the terminal after connecting to it !! */

