#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <fstream>
#include <openssl/sha.h>
using namespace std;
#include "utils.h"

// Defines
#define BACKLOG 1280  // Maximum number of pending connections
#define MAX_PACKET_SIZE 2048

// Global vars
vector<int> client_fds;  // Vector to store client file descriptors

// Functions prototypes
void handleConnections(int server_fd);
void handleClient(int client_fd);

void handleConnections(int server_fd)
{
    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd < 0) {
            cerr << "Accept failed" << endl;
            continue;
        }

        cout << "Client connected!" << endl;

        // Store client file descriptor
        client_fds.push_back(client_fd);

        // Handle client in a separate thread
        thread client_thread = thread(handleClient, client_fd);
        client_thread.detach();  // Detach the thread to run independently
    }
}

// Function to handle new connections with threads
void handleClient(int client_fd)
{
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, &buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break;  // Exit if no data received or error
        
        string command(buffer);
        if (command == "Sair") {
            cout << "Client requested to exit" << endl;
            break;  // Exit the loop if client sends "Sair"
        }
        else if (command.rfind("Chat ", 0) == 0) {
            string chat_message = command.substr(5);
            cout << "Chat message: " << chat_message << " from " << client_fd << endl;
        }
        else if (command.rfind("Arquivo ", 0) == 0) {
            string filename = command.substr(8);
            if (!fileExists(filename)) {
                send(client_fd, "ERRO", 5, 0);
                continue;
            }
            
            string hash = SHA256(filename);
            auto file_data = readFile(filename);
            int file_size = file_data.size();

            int file_size_n = htonl(file_size);

            send(client_fd, "OKAY", 5, 0);  // Acknowledge file request
            send(client_fd, &file_size_n, sizeof(file_size_n), 0);  // Send file size
            send(client_fd, hash.c_str(), 65, 0);  // Send file hash
            sendAll(client_fd, file_data.data(), file_size);  // Send file data
        }
    }
    close(client_fd);
}

int main() {
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Socket creation failed" << endl;
        return -1;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Setsockopt failed" << endl;
        close(server_fd);
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Bind failed" << endl;
        close(server_fd);
        return -1;
    }

    // Listen for connections
    if (listen(server_fd, BACKLOG) < 0) {
        cerr << "Listen failed" << endl;
        close(server_fd);
        return -1;
    }

    cout << "Servidor escutando na porta 8080..." << endl;

    // Handle incoming connections
    thread server_thread(handleConnections, server_fd);
    server_thread.detach();  // Detach the server thread to run independently

    // Wait for input message to broadcast to all clients
    while (true) {
        string message = "";
        cout << "Digite a mensagem para transmitir (ou 'Sair' para sair): ";
        getline(cin, message);

        if (message == "Sair") {
            cout << "Saindo do servidor..." << endl;
            for (int client_fd : client_fds) {
                send(client_fd, message.c_str(), message.size(), 0);
            }
            break;
        }
        
        message = message + "\0";  // Prefix with "Chat" for consistency

        // Broadcast message to all clients
        for (int client_fd : client_fds) {
            send(client_fd, "Chat", 5, 0);
            send(client_fd, message.c_str(), 1024, 0);
        }
    }

    // Clean up
    close(server_fd);
    return 0;
}