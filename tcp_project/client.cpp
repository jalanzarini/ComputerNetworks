#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
using namespace std;

// Receive message thread
void receiveMessage(int client_fd) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received <= 0) {
            cout << "Server disconnected" << endl;
            close(client_fd);
            exit(0);
        }

        cout << "Server response: " << buffer << endl;
    }
}

int main() {
    // Create socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        cerr << "Socket creation failed" << endl;
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Connect to localhost

    // Connect to server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Connection failed" << endl;
        close(client_fd);
        return -1;
    }

    cout << "Connected to server!" << endl;

    // Start a thread to receive messages from the server
    thread receiveThread(receiveMessage, client_fd);
    receiveThread.detach();  // Detach the thread to run independently

    // Communication loop
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        // Get user input
        cout << "Enter message (or 'exit' to quit): ";
        cin.getline(buffer, sizeof(buffer) - 1);
        
        // Check if user wants to exit
        if (strcmp(buffer, "exit") == 0) {
            cout << "Exiting..." << endl;
            break;
        }
        
        // Send message to server
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Clean up
    close(client_fd);
    return 0;
}