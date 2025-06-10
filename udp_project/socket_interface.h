#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 64000
#define CHECKSUM_SIZE 32

struct DataPacket{
	char data[MAX_BUFFER_SIZE+1];
	char checksum[CHECKSUM_SIZE+1];
	int sequence;
};

class NetworkAddress{
private:
	struct sockaddr_in network_address;
	socklen_t network_address_len;
	std::string ip;
	int port;

public:
	NetworkAddress();
	NetworkAddress(std::string ip, int port);
	~NetworkAddress();
	
	inline struct sockaddr_in GetNetworkAddress(){ return network_address; }
	inline socklen_t GetNetworkAddressLen(){ return network_address_len; }
	inline std::string GetIP(){ return ip; }
	inline int GetPort(){ return port; }
	
	void Create(std::string ip, int port);
};

class MySocket{
private:
	int socketfd;
	struct sockaddr_in host_address;
	
public:
	MySocket();
	~MySocket();  
	
	bool Create(NetworkAddress addr);
	void SendMessage(std::string message, NetworkAddress addr);
	std::string RecvMessage(NetworkAddress* addr);
	void SendTo(DataPacket packet, NetworkAddress addr);
	DataPacket Recv(NetworkAddress* addr);
	void SetRecvTimeout(int seconds);
};
