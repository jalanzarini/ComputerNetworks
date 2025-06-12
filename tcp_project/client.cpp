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
        memset(buffer, 0, sizeof(buffer));
        recv(client_fd, buffer, 5, 0);
        if (strncmp(buffer, "OKAY", 2) == 0)
        {
            int file_size;
            recv(client_fd, &file_size, sizeof(file_size), 0);
            file_size = ntohs(file_size);

            char hash[65] = {};
            recv(client_fd, hash, 65, 0);

            vector<char> file_data(file_size);
            rcvAll(client_fd, file_data.data(), file_size);
            
            string filename = "recebido";  // Extract filename from input
            ofstream file(filename, ios::binary);
            
            file.write(file_data.data(), file_size);
            file.close();

            string local_hash = SHA256(filename);
            if (hash != local_hash) {
                cout << "Hash mismatch! Received: " << hash << ", Expected: " << local_hash << endl;
                continue;
            }
        } 
        else if(strncmp(buffer, "ERRO", 4) == 0){
            cout << "Arquivo não encontrado." << endl;
        }
        else if (strncmp(buffer, "Chat", 4) == 0) {
            char msg[1024];
            recv(client_fd, msg, sizeof(msg), 0);
            cout << "Chat message: " << msg << endl;
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
        char buffer[1024];
        getline(cin, input);
        memcpy(buffer, input.c_str(), 1024);

        // Check if user wants to exit
        if (input == "Sair") {
            cout << "Saindo..." << endl;
            send(client_fd, buffer, sizeof(buffer), 0);
            break;
        } else if (input.rfind("Chat ", 0) == 0){
            // Send message to server
            send(client_fd, buffer, sizeof(buffer), 0);
        } else if (input.rfind("Arquivo ", 0) == 0) {
            send(client_fd, buffer, sizeof(buffer), 0);
        } else {
            cout << "Comando desconhecido. Use 'Chat <mensagem>' ou 'Arquivo <nome_do_arquivo>'." << endl;
            continue;
        }            
    }

    // Clean up
    close(client_fd);
    return 0;
}