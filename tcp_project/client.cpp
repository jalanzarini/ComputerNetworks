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

// Receive message thread
void receiveMessage(int client_fd) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        cout << "Opa beleza." << endl;
        cout << bytes_received << endl;
        string msg;
        msg += buffer;
        string delimiter = " ";
        string request = msg.substr(0, msg.find(delimiter));
        string temp = msg.substr(msg.find("Tamanho: ") + 9);
        cout << temp << endl;
        int buf_size = stoi(temp.substr(0, temp.find("\n")));
        msg = "";

        while(buf_size > 0) {
            cout << "Buffer size: " << buf_size << endl;
            msg += buffer;
            buf_size -= sizeof(buffer) - 1;
            bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        }

        if (bytes_received <= 0) {
            cout << "Server disconnected" << endl;
            close(client_fd);
            exit(0);
        }

        if(request == "Chat") {
            string chat_message = msg.substr(msg.find(delimiter) + 1);
            cout << "Broadcasted message: " << chat_message << endl;
        } else if(request == "Sair") {
            cout << "Server has closed the connection." << endl;
            close(client_fd);
            exit(0);
        }
        else if (request == "OK"){
            string data = msg.substr(msg.find("data:") + 5);
            ofstream file("received_file.txt");
            if (file.is_open()) {
                file << data;
                file.close();
                cout << "File received and saved as 'received_file.txt'" << endl;
            } else {
                cerr << "Error opening file for writing." << endl;
            }
        }
        else if (request == "ERRO_ARQUIVO_NAO_ENCONTRADO") {
            cout << "Error: File not found on server." << endl;
        } else {
            cout << "Unknown request: " << request << endl;
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
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        // Get user input
        cout << "Enter message (or 'exit' to quit): ";
        cin.getline(buffer, sizeof(buffer) - 1);
        
        // Check if user wants to exit
        if (strcmp(buffer, "Sair") == 0) {
            cout << "Saindo..." << endl;
            send(client_fd, buffer, strlen(buffer), 0);
            break;
        }
        
        // Send message to server
        send(client_fd, buffer, strlen(buffer), 0);
    }

    // Clean up
    close(client_fd);
    return 0;
}