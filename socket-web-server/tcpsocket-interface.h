int tcpsocket_create(const char* hostname, int port);
void tcpsocket_close(int socket_fd);
int tcpsocket_connect(int socket_fd, const char* hostname, int port);
int tcpsocket_listen(int socket_fd, int connections);
int tcpsocket_accept(int socket_fd);
int tcpsocket_send(int socket_fd, void* buffer, int length);
int tcpsocket_receive(int socket_fd, void* buffer, int length);
