#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
using namespace std;

string SHA256(const string& path);
vector<char> readFile(const string& filename);

bool fileExists(const string& filename);
void sendAll(int socket, const char* buffer, size_t length);
void rcvAll(int socket, char* buffer, size_t length);

#endif // UTILS_H