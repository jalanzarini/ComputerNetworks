#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <fstream>
using namespace std;
#include "utils.h"

// Receive message thread
void receiveMessage(int client_fd) {
    while (true) {
        char buffer[1024];
        recv(client_fd, buffer, sizeof(buffer), 0);
        if (strncmp(buffer, "OK", 2) == 0)
        {
            cout << "chegou 1"<< buffer << endl;
            uint64_t file_size;
            rcvAll(client_fd, (char*)&file_size, sizeof(file_size));
            
            cout << "chegou 2" << file_size << endl;
            
            char hash[65] = {};
            rcvAll(client_fd, hash, 64);

            cout << "chegou 3" << hash << endl;

            vector<char> file_data(file_size);
            rcvAll(client_fd, file_data.data(), file_size);
            
            cout << "chegou 4" << endl;

            string filename = "recebido";  // Extract filename from input
            ofstream file(filename, ios::binary);
            
            cout << "chegou 5" << endl;

            file.write(file_data.data(), file_size);
            file.close();

            cout << "chegou 6" << endl;

            string local_hash = SHA256(filename);
            if (hash != local_hash) {
                cout << "Hash mismatch! Received: " << hash << ", Expected: " << local_hash << endl;
                continue;
            }
        } else if (strncmp(buffer, "Chat ", 5) == 0) {
            cout << "Chat message: " << (buffer + 5) << endl;
        } 
        else if (strncmp(buffer, "Sair", 4) == 0) {
            cout << "Server has closed the connection." << endl;
            close(client_fd);
            exit(0);  // Exit the thread
        } else {
            cout << "Received unknown command: " << buffer << endl;
        }
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
    string input;
    while (true) {        
        // Get user input
        cout << "Insira a mensagem (ou 'Sair' para sair): ";
        getline(cin, input);

        // Check if user wants to exit
        if (input == "Sair") {
            cout << "Saindo..." << endl;
            send(client_fd, input.c_str(), input.size(), 0);
            break;
        } else if (input.rfind("Chat ", 0) == 0){
            // Send message to server
            send(client_fd, input.c_str(), input.size(), 0);
        } else if (input.rfind("Arquivo ", 0) == 0) {
            send(client_fd, input.c_str(), input.size(), 0);
        } else {
            cout << "Comando desconhecido. Use 'Chat <mensagem>' ou 'Arquivo <nome_do_arquivo>'." << endl;
            continue;
        }            
    }

    // Clean up
    close(client_fd);
    return 0;
}