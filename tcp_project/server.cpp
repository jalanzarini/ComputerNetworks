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
        
        int bytes_received = recv(client_fd, &buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            cout << "Client disconnected" << endl;
            break;
        }

        string msg(buffer, bytes_received);
        string delimiter = " ";
        string request = msg.substr(0, msg.find(delimiter));

        if (request == "Sair") {
            cout << "Client requested to exit" << endl;
            break;  // Exit the loop if client sends "Sair"
        }
        else if (request == "Arquivo") {
            string filename = msg.substr(msg.find(delimiter) + 1);
            ifstream file(filename);

            if(file.is_open()) {
                string file_content;

                // Read file content
                string line;
                while (getline(file, line)) {
                    file_content += line + "\n";  // Append each line with a newline character
                }

                file.close();

                // Send status message
                string status = "OK";

                string metadata = "Arquivo: " + filename + ", Tamanho: " + "temp" + "\n";

                // Create hash of the file content
                unsigned char hash[SHA256_DIGEST_LENGTH];
                SHA256_CTX sha256;
                SHA256_Init(&sha256);
                SHA256_Update(&sha256, file_content.c_str(), file_content.size());
                SHA256_Final(hash, &sha256);
                
                // Send hash to client
                string hash_message = "Hash: " + string(reinterpret_cast<char*>(hash), SHA256_DIGEST_LENGTH);

                string packet = status + " \n" + metadata + " \n" + hash_message + " \n" + "data:";

                int size = packet.size() + file_content.size();
                packet.replace(packet.find("temp"), 4, to_string(size));

                cout << "Sending file: " << filename << " to client." << endl;
                send(client_fd, packet.c_str(), packet.size(), 0);
                for(int i = 0; i*MAX_PACKET_SIZE <= file_content.size(); i++){
                    cout << file_content.substr(i*MAX_PACKET_SIZE, MAX_PACKET_SIZE) << endl;
                    send(client_fd, file_content.substr(i*MAX_PACKET_SIZE, MAX_PACKET_SIZE).c_str(), MAX_PACKET_SIZE, 0);
                }

            } else {
                string error_message = "ERRO_ARQUIVO_NAO_ENCONTRADO";
                send(client_fd, error_message.c_str(), error_message.size(), 0);
            }
        }
        else if (request == "Chat") {
            string chat_message = msg.substr(msg.find(delimiter) + 1);
            cout << "Chat message: " << chat_message << " from " << client_fd << endl;
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
    string message;
    while (true) {
        cout << "Digite a mensagem para transmitir (ou 'Sair' para sair): ";
        getline(cin, message);

        if (message == "Sair") {
            cout << "Saindo do servidor..." << endl;
            message += " ";
            for (int client_fd : client_fds) {
                send(client_fd, message.c_str(), message.size(), 0);
            }
            break;
        }
        
        message = "Chat " + message;  // Prefix with "Chat" for consistency

        // Broadcast message to all clients
        for (int client_fd : client_fds) {
            send(client_fd, message.c_str(), message.size(), 0);
        }
    }

    // Clean up
    close(server_fd);
    return 0;
}