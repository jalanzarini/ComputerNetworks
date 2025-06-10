#include "socket_interface.h"

#include <cstring>
#include <arpa/inet.h>
#include "crc32.h"
#include <time.h>

NetworkAddress::NetworkAddress(){}
NetworkAddress::NetworkAddress(std::string ip, int port){
	Create(ip, port);
}
NetworkAddress::~NetworkAddress(){}

void NetworkAddress::Create(std::string ip, int port){
	this->ip = ip;
	this->port = port;
	
	network_address.sin_family = AF_INET;
	inet_aton(ip.c_str(), &network_address.sin_addr);
	network_address.sin_port = htons(port);
	network_address_len = sizeof(network_address);
}

MySocket::MySocket(){}
MySocket::~MySocket(){}

bool MySocket::Create(NetworkAddress addr){
	socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socketfd == -1) return false;
	
	struct sockaddr_in host_addr = addr.GetNetworkAddress();
	socklen_t host_addr_len = addr.GetNetworkAddressLen();
	if(bind(socketfd, (struct sockaddr*)&host_addr, host_addr_len) < 0) return false;
	return true;
}

void MySocket::SendMessage(std::string message, NetworkAddress addr){
	int it = 0, len = message.length();
	DataPacket packet;
	while(len > 0){	
		std::string packet_data = message.substr(it*MAX_BUFFER_SIZE, MAX_BUFFER_SIZE);
		strcpy(packet.data, packet_data.c_str());
		strcpy(packet.checksum, crc32_remainder(packet_data).c_str());
		packet.sequence = htons(it);
		int ack = 0;
		while(ack == 0){
			SendTo(packet, addr);
			printf("[PACKET] Sent: Sequence %d\n", it);
			DataPacket ack_packet;
			SetRecvTimeout(1);
			ack_packet = Recv(nullptr);
			SetRecvTimeout(0);
			if(std::string(ack_packet.data) == "ACK" && ntohs(ack_packet.sequence) == it) ack = 1;
		}
		printf("[PACKET] Received: Sequence ACK %d\n", it);
		len -= MAX_BUFFER_SIZE;
		it++;
	}
	memset(&packet, 0, sizeof(packet));
	strcpy(packet.data, "FIN");
	SendTo(packet, addr);
	printf("[FIN] Sent: End of Transmission\n");
}

std::string MySocket::RecvMessage(NetworkAddress* addr){
	NetworkAddress remote_address;
	DataPacket packet;
	std::string ans = "";
	srand(time(NULL));
	for(packet = Recv(&remote_address); std::string(packet.data) != "FIN"; packet = Recv(&remote_address)){
		if(crc32_check(std::string(packet.data), std::string(packet.checksum)) == 0){
			printf("[PACKET] Corrupted: Sequence %d\n", ntohs(packet.sequence));
			continue;
		}
		if(rand()%100 <= 1){
			printf("[PACKET] Lost: Sequence %d\n", ntohs(packet.sequence));
			continue;
		}
		printf("[PACKET] Received: Sequence %d\n", ntohs(packet.sequence));
		ans += packet.data;
		DataPacket ack_packet;
		strcpy(ack_packet.data, "ACK");
		strcpy(ack_packet.checksum, crc32_remainder("ACK").c_str());
		ack_packet.sequence = packet.sequence;
		SendTo(ack_packet, remote_address);
		printf("[PACKET] Sent: Sequence ACK %d\n", ntohs(packet.sequence));
	}
	if(addr != nullptr){
		addr->Create(remote_address.GetIP(), remote_address.GetPort());
	}
	printf("[FIN] Received: End of Transmission\n");
	return ans;
}

void MySocket::SendTo(DataPacket packet, NetworkAddress addr){
	struct sockaddr_in remote_addr = addr.GetNetworkAddress();
	socklen_t remote_addr_len = addr.GetNetworkAddressLen();
	
	sendto(socketfd, &packet, sizeof(packet), 0, (struct sockaddr*)&remote_addr, remote_addr_len);
}

DataPacket MySocket::Recv(NetworkAddress* addr){
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len = sizeof(remote_addr);
	
	int bytes_recv;
	DataPacket packet;
	bytes_recv = recvfrom(socketfd, &packet, sizeof(packet), MSG_WAITALL, (struct sockaddr*)&remote_addr, &remote_addr_len);	
	
	if(bytes_recv < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
		strcpy(packet.data, "TIMEO");
		return packet; 
	}

	if(addr != nullptr){
		char ip[1024];
		strcpy(ip, inet_ntoa(remote_addr.sin_addr));
		std::string ipstr = ip;	
		int port = ntohs(remote_addr.sin_port);	
		addr->Create(ipstr, port);
	}
	return packet;
}

void MySocket::SetRecvTimeout(int seconds){
	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = 0;
	setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
}
