#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "tcpsocket-interface.h"
#include <errno.h>

static int socket_fd;
static char request_buffer[21234];
static char message_buffer[21234];

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
    memset(message_buffer, 0, sizeof(message_buffer));
    if(get_request){
      char filename[2123] = "./pages";
      char subfilename[2123];
      memset(subfilename, 0, sizeof(subfilename));
      memcpy(subfilename, request_buffer+4, strchr(request_buffer+4, ' ')-(request_buffer+4));
      if(strcmp(subfilename, "/") == 0) strcat(subfilename, "main.html");
      strcat(filename, subfilename);
      printf("%s %s\n", filename, subfilename);
      FILE *fptr;
      fptr = fopen(filename, "rb");
      printf("%d\n", fptr == NULL);
      if(fptr == NULL){
        fptr = fopen("./pages/404.html", "rb");
        fseek(fptr, 0, SEEK_END);
        long file_bytes = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        char content_length[2123];
        sprintf(content_length, "Content-Length: %ld", file_bytes);
        strcat(message_buffer, "HTTP/1.1 404 Not Found\n");
        strcat(message_buffer, "Content-Type: text/html\n");
        strcat(message_buffer, content_length);
        strcat(message_buffer, "\n\r\n");
        char file_buffer[2123];
        while(fgets(file_buffer, sizeof(file_buffer), fptr)){
          strcat(message_buffer, file_buffer);
        }
        int bytes = tcpsocket_send(client_fd, message_buffer, strlen(message_buffer));
        fclose(fptr);
      }
      else{
        fseek(fptr, 0, SEEK_END);
        long file_bytes = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);
        printf("file size: %d\n", file_bytes);
        char content_length[2123];
        sprintf(content_length, "Content-Length: %ld", file_bytes);
        strcat(message_buffer, "HTTP/1.1 200 OK\n");
        strcat(message_buffer, "Content-Type: image/png\n");
        strcat(message_buffer, content_length);
        strcat(message_buffer, "\n\r\n");
        int bytes = tcpsocket_send(client_fd, message_buffer, strlen(message_buffer));
        char file_buffer[2123456];
        while(fgets(file_buffer, sizeof(file_buffer), fptr)){
          int bytes = tcpsocket_send(client_fd, file_buffer, strlen(file_buffer));
        }
        fclose(fptr);
      }
    }
    else{
      strcat(message_buffer, "HTTP/1.1 501 Not Implemented\n");
      strcat(message_buffer, "Content-Type: text/html; charset=UTF-8\n");
      strcat(message_buffer, "Content-Length: 202\n\n");
      strcat(message_buffer, "<html>\n<head>\n<title>Function Not Implemented</title>\n</head>\n<body>\nYour request can not be completed because this functionality is currently under development.\n</body>\n</html>\n");
      int bytes = tcpsocket_send(client_fd, message_buffer, sizeof(message_buffer));
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
