#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tcpsocket-interface.h"
#include <errno.h>

static int socket_fd;
static char request_buffer[21234];

void send_file(int socket_fd, FILE* fptr, const char* status_code, const char* content_type){
  char header_buffer[21234];
  memset(header_buffer, 0, sizeof(header_buffer));
 
  //Get file size
  fseek(fptr, 0, SEEK_END);
  long file_bytes = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  char content_length[2123];
  sprintf(content_length, "Content-Length: %ld\n", file_bytes);

  char status[2123];
  sprintf(status, "HTTP/1.1 %s\n", status_code);

  char type[2123];
  sprintf(type, "Content-Type: %s\n", content_type);

  strcat(header_buffer, status);
  strcat(header_buffer, type);
  strcat(header_buffer, content_length);
  strcat(header_buffer, "\r\n");
  tcpsocket_send(socket_fd, header_buffer, strlen(header_buffer));
  
  int size;
  char file_buffer[2123];
  while(size = fread(file_buffer, sizeof(char), sizeof(file_buffer), fptr)){
    tcpsocket_send(socket_fd, file_buffer, size);
  }
}

void get_handler(int socket_fd){
  // Parse filename to desired path
  char filename[2123];
  char subfilename[2123];
  
  memset(filename, 0, sizeof(filename));
  
  memset(subfilename, 0, sizeof(subfilename));
  memcpy(subfilename, request_buffer+4, strchr(request_buffer+4, ' ')-(request_buffer+4));
  strcat(subfilename, "\0");

  if(subfilename[strlen(subfilename)-1] == '/') strcat(subfilename, "main.html");
  
  int isImage = 0;
  if(strstr(subfilename, ".png") != NULL) strcat(filename, "./images"), isImage = 1;
  else strcat(filename, "./pages");
  strcat(filename, subfilename);

  // Open file and build response
  FILE* fptr;
  fptr = fopen(filename, "rb");
  if(fptr != NULL){
    send_file(socket_fd, fptr, "200 OK", isImage ? "image/png": "text/html");
    fclose(fptr);
  }
  else{
    fptr = fopen("./pages/404.html", "rb");
    send_file(socket_fd, fptr, "404 Not Found", "text/html");
    fclose(fptr);
  }
}

void* client_handler(void* arg){
  int* p_remote_fd = (int*)arg;
  int client_fd = *p_remote_fd;
  free(p_remote_fd);
  printf("Client %d connected\n", client_fd);

  while(1){
    // Get request headers
    char tmp_buffer[2123];
    int req_end = 0;
    int next_req_start = 0, next_req_len = 0;
    memset(tmp_buffer, 0, sizeof(tmp_buffer));
    while(!req_end){
      int bytes = tcpsocket_receive(client_fd, tmp_buffer, sizeof(tmp_buffer));
      if(bytes == 0 || bytes == -1) return 0;
      char* endofstream = strstr(tmp_buffer, "\n\r\n");
      if(endofstream != NULL){
        req_end = 1;
        int last_char = endofstream-tmp_buffer+2;
        next_req_start = last_char+1;
        next_req_len = strlen(tmp_buffer)-last_char;
        memcpy(request_buffer, tmp_buffer, last_char+1);
      }
      else strcat(request_buffer, tmp_buffer);
    }
    printf("%s", request_buffer);

    // Process request
    int get_request = 1;
    char* get_pos = strstr(request_buffer, "GET ");
    if(get_pos != request_buffer) get_request = 0;

    // Send Response
    if(get_request) get_handler(client_fd);
    else{
      FILE* fptr;
      fptr = fopen("./pages/501.html", "rb");
      send_file(client_fd, fptr, "501 Not Implemented", "text/html");
    }
    // Adjust next request header
    memset(request_buffer, 0, sizeof(request_buffer));
    memcpy(request_buffer, tmp_buffer+next_req_start, next_req_len);
  }
}

void* connection_handler(void* arg){
  while(1){
    int* remote_fd = (int*)malloc(sizeof(int));
    *remote_fd = tcpsocket_accept(socket_fd);
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, client_handler, remote_fd);
    pthread_detach(client_thread);
  }
}

int main(int argc, char* argv[]){
  socket_fd = tcpsocket_create("127.0.0.1", 1234);
  tcpsocket_listen(socket_fd, 10);
  
  pthread_t connection_thread;
  pthread_create(&connection_thread, NULL, connection_handler, NULL);
  pthread_detach(connection_thread);

  while(1);
  return 0;
}
