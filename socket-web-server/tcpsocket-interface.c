#include "tcpsocket-interface.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int tcpsocket_create(const char* hostname, int port){
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_fd == -1) return -1;

  struct sockaddr_in socket_addr;
  inet_aton(hostname, &socket_addr.sin_addr);
  socket_addr.sin_family = AF_INET;
  socket_addr.sin_port = htons(port);
  if(bind(socket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) < 0) return -1;

  return socket_fd;
}

void tcpsocket_close(int socket_fd){
  close(socket_fd);
}

int tcpsocket_connect(int socket_fd, const char* hostname, int port){
  struct sockaddr_in remote_addr;
  inet_aton(hostname, &remote_addr.sin_addr);
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(port);
  return connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
}

int tcpsocket_listen(int socket_fd, int connections){
  return listen(socket_fd, connections);
}

int tcpsocket_accept(int socket_fd){
  struct sockaddr_in remote_addr;
  socklen_t addr_len = sizeof(remote_addr);
  int remote_fd = accept(socket_fd, (struct sockaddr*)&remote_addr, &addr_len);
  return remote_fd;
}

int tcpsocket_send(int socket_fd, void* buffer, int length){
  return send(socket_fd, buffer, length, 0);
}

int tcpsocket_receive(int socket_fd, void* buffer, int length){
  return recv(socket_fd, buffer, length, 0);
}
