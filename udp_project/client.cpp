#include <iostream>

#include <string>
#include <fstream>
#include "socket_interface.h"

bool SaveFileContent(std::string filepath, std::string text){
	std::ofstream file(filepath);
	if(!file.is_open()) return false;
	file << text;	
	file.close();
	return true;
}

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Use this as\n./ClientUDPFTP [port] [file]\n");
		return 0;
	}
	MySocket host;
	NetworkAddress hostAddress = NetworkAddress("127.0.0.1", 1235);	
	NetworkAddress remoteAddress = NetworkAddress("127.0.0.1", std::stoi(argv[1]));	
	host.Create(hostAddress);
	host.SendMessage("GET " + std::string(argv[2]), remoteAddress);	
	std::string response = host.RecvMessage(nullptr);
	if(response.substr(0, 11) == "File Found\n"){
		std::string file_content = response.substr(11, response.length() - 11);
		SaveFileContent("built.txt", file_content);	
	}
	printf("%s", response.substr(0, response.find('\n')+1).c_str());
	return 0;
}
