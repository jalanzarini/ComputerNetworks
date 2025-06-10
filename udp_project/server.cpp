#include <iostream>

#include <string>
#include <fstream>
#include "socket_interface.h"

std::string GetFileContent(std::string filepath){
	std::string ans;
	std::ifstream file(filepath);
	if(!file.is_open()) return "";
	char c;
	while((c = file.get()) != EOF){
		ans += c;
	}
	file.close();
	return ans;
}

bool FileExists(std::string filepath){
	std::ifstream file(filepath);
	if(!file.is_open()) return false;
	file.close();
	return true;	
}

int main(int argc, char *argv[]){
	MySocket host;
	NetworkAddress hostAddress = NetworkAddress("127.0.0.1", 1234);	
	NetworkAddress remoteAddress;
	host.Create(hostAddress);
	while(1){
		std::string request = host.RecvMessage(&remoteAddress);
		if(request.substr(0, 4) == "GET "){
			std::string filepath = request.substr(4, request.length()-4);
			if(!FileExists(filepath)){
				host.SendMessage("File Not Found\n", remoteAddress);
			}
			else{
				std::string text = "File Found\n";
				text += GetFileContent(filepath);
				host.SendMessage(text, remoteAddress);
			}
		}
	}
	return 0;
}
