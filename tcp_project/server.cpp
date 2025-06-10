#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
using namespace std;

// Defines
#define BACKLOG 1280  // Maximum number of pending connections

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
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            cout << "Client disconnected" << endl;
            break;
        }

        cout << "Received: " << buffer << " from " << client_fd << endl;

        // Echo back to client
        send(client_fd, buffer, bytes_received, 0);
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

    cout << "Server listening on port 8080..." << endl;

    // Handle incoming connections
    thread server_thread(handleConnections, server_fd);
    server_thread.detach();  // Detach the server thread to run independently

    // Wait for input message to broadcast to all clients
    string message;
    while (true) {
        cout << "Enter message to broadcast (or 'exit' to quit): ";
        getline(cin, message);
        if (message == "exit") {
            cout << "Exiting server..." << endl;
            break;
        }
        // Broadcast message to all clients
        for (int client_fd : client_fds) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
        cout << "Message broadcasted: " << message << endl;
    }

    // Clean up
    close(server_fd);
    return 0;
}