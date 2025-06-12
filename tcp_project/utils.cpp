#include "utils.h"
#include <openssl/sha.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>

bool fileExists(const string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

vector<char> readFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Could not open file: " + filename);
    }
    
    vector<char> buffer((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    return buffer;
}

string SHA256(const string& path) {
    ifstream file(path, ios::binary);
    if(!file) return "";

    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    char buff[4096];
    while(file.read(buff, sizeof(buff))) {
        SHA256_Update(&ctx, buff, file.gcount());
    }
    SHA256_Update(&ctx, buff, file.gcount());

    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);

    char out[65];
    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(out + (i * 2), "%02x", hash[i]);
    }
    out[64] = 0;
    return string(out);
}

void sendAll(int socket, const char* buffer, size_t length) {
    size_t total_sent = 0;
    while (total_sent < length) {
        ssize_t sent = send(socket, buffer + total_sent, length - total_sent, 0);
        if (sent == -1) {
            throw runtime_error("Failed to send data");
        }
        total_sent += sent;
    }
}

void rcvAll(int socket, char* buffer, size_t length) {
    size_t total_received = 0;
    while (total_received < length) {
        ssize_t received = recv(socket, buffer + total_received, length - total_received, 0);
        if (received <= 0) {
            throw runtime_error("Failed to receive data or connection closed");
        }
        total_received += received;
    }
}