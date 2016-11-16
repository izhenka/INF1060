#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

char* read_next_job(FILE* file, int* eof_reached);
int create_socket(int port);
void terminate(int signum);
int accept_connection(int server_sock);
int talk_with_client(FILE* file, int client_sock);
ssize_t send_next_job(FILE* file, int client_sock, int* eof_reached);

#define INCOM_MSG_SIZE 2
#define REPLY_MSG_SIZE 258

int running = 1; //continue to accept connections
int client_running = 0; //continue to exchange with client


/*
* Server!
*/
int main(int argc, char const *argv[]) {

  //Cntrl + c handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = terminate;

  if (sigaction(SIGINT, &sa, NULL) != 0) {
    perror("sigaction()");
    exit(EXIT_FAILURE);
  }

  //arguments check
  if (argc != 3) {
		fprintf(stderr, "Usage: %s <filename> <port>\n", argv[0]);
		return -1;
	}

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
    return -1;
  }

  int port = (int) strtol(argv[2], NULL, 10);
  if(port == 0){
    perror("Wrong port number");
		return -1;
  }

  //connections handling
  int sock = create_socket(port);
  if (sock == -1){
    exit(EXIT_FAILURE);
  }
  //printf("Socket created successfully, num: %d\n", sock);


  while (running){

    //accept connection
    printf("Server: waiting for connections...\n");
    int client_sock = accept_connection(sock);
    client_running = 1;
    //printf("New connection accepted. client_sock: %d\n", client_sock);
    if (client_sock == -1){
      continue;
    }

    //exchanging with client untill its done
    while(client_running){
      //printf("new talk_with_client...\n");
      int res = talk_with_client(file, client_sock);
      switch (res) {
        case 0: printf("Exchange succeed!\n"); break;
        case 1: printf("Client terminated normally\n"); client_running = 0; break;
        case 2: printf("Client terminated on error\n"); client_running = 0; break;
        case 3: printf("Client disconnected\n"); client_running = 0; break;
        case 5: printf("Client terminated normally after request from server\n");
                client_running = 0; running = 0; break;
        default: printf("Exchange failed! Trying again...\n");
      }
    }
    //printf("done! talk_with_client...\n");

    close(client_sock);
  }


  close(sock);
  fclose(file);
  return 0;
}

/* Reads next job parameters from file into input parameters
*  returns -1 on error (f.eks end of file is reached), else 0
*/
char* read_next_job(FILE* file, int* eof_reached){

  char* res_message;

  int bytes_read = 0;

  char work_type;
  unsigned char message_len = 0;

  bytes_read = fread(&work_type, sizeof(char), 1, file);
  if (bytes_read < 1){ //end of file
    //quit-message
    res_message = malloc(3);
    work_type = 'Q';
    memmove(res_message, &work_type, 1);
    memmove(res_message + 1, &message_len, 1);
    res_message[2] = '\0';
    *eof_reached = 1;
    return res_message;
  }
  //printf("work_type: %c\n", work_type);

  fread(&message_len, sizeof(unsigned char), 1, file);
  //printf("message_len: %u\n", message_len);

  char message[256] = {0};
  fread(message, sizeof(char), message_len, file);
  //printf("message: %s\n", message);

  res_message = malloc(2 + message_len +1);
  memmove(res_message, &work_type, 1);
  memmove(res_message + 1, &message_len, 1);
  memmove(res_message + 2, message, message_len + 1);

  // printf("res_message: %s, len: %lu\n", res_message, strlen(res_message));
  // printf("res_message[0]: %c\n", res_message[0]);
  // printf("res_message[1]: %d\n", res_message[1]);
  // printf("res_message[2]: %s\n", (res_message + 2));

  return res_message;
}

/* Creates socket and return its index
* returns -1 on error
*/
int create_socket(int port){

  //creating socket
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		perror("socket()");
		return -1;
	}
  //printf("Socket is set!\n");

  //setting internet address
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  //printf("Sockaddr_in is set %u!\n", server_addr.sin_addr.s_addr);

  //setting reuse of address
  int yes = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) {
    perror("setsockopt()");
    close(sock);
    return -1;
  }

  //binding address to socket
  if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0){
    perror("bind()");
    close(sock);
    return -1;
  }
  //printf("Address is binded!\n");

  //activate listening to connections
  if (listen(sock, SOMAXCONN) != 0) {
  //if (listen(sock, 0) != 0) {
		perror("listen()");
		close(sock);
		return -1;
	}
  //printf("Listening is on!\n");

  return sock;

}

/* SIGINT handler
* terminates connections acception
*/
void terminate(int signum)
{
	printf("\nTerminating server (signal %d).\n", signum);
	running = 0;
  client_running = 0;
}

/* Accept client connection
* returns clients socket
* returns -1 on error
*/
int accept_connection(int server_sock){

  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  socklen_t addr_len = sizeof(client_addr);

  int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
  if (client_sock == -1) {
    if (errno != EINTR){
      perror("accept()");
    }
    return -1;
  }

  char *client_ip = inet_ntoa(client_addr.sin_addr);
  printf("Client connected! IP: %s\n", client_ip);

  return client_sock;

}

/* Provides exchage with client according to protocol:
*   receive:
*     each message of 2 bytes: char and unsigned char
*     'G'0 	- get 1 job
*     'A'0 	- get all jobs
*     'N'x  - get x jobs
*     'T'0 	- client terminates normally
*     'E'0	- client terminates with error
*     'Q'0 	- client terminates normally after request from server. Server can terminate.
*
*   send:
*   ... (se oppgavetekst)
*   byte 1: job type (O/E/Q)
*   byte 2: message length
*   rest:   message
*
* returns:
*    0 exchange succeed
*    1 client terminated normally
*    2 client terminated on error
*    3 client disconnected
*    4 end of file reached, server should terminate
*    5 client terminated normally after request from server
*   -1 on error
*/
int talk_with_client(FILE* file, int client_sock){

  //receiving
  ssize_t ret;
  char buf[INCOM_MSG_SIZE] = { 0 };
  //printf("recv from client ...\n");
  ret = recv(client_sock, buf, INCOM_MSG_SIZE, 0);
  //printf("reicived from client buf: %s, len buf: %lu, bytes received %zd:\n", buf, strlen(buf), ret);
  if (ret == 0) {
    printf("Client disconnected.\n");
    return 3;
  } else if (ret == -1) {
    perror("recv()");
    return -1;
  }
  char req_type = buf[0];
  unsigned int number_jobs = buf[1];
  printf("Message from client: request type: %c, number jobs: %d\n", req_type, number_jobs);

  //sending
  if (req_type == 'G'){
    int eof_reached = 0;
    ret = send_next_job(file, client_sock, &eof_reached);
    if (ret == -1){
      perror("send()");
      return -1;
    }
    return 0;

  } else if (req_type == 'N'){

    int eof_reached = 0;
    for (int i = 0; i < (int)number_jobs; i++){
      ret = send_next_job(file, client_sock, &eof_reached);
      if (ret == -1){
        perror("send()");
        return -1;
      }
      if (eof_reached){
        break;
      }
    }
    return 0;
  } else if (req_type == 'A'){
    int eof_reached = 0;
    while(!eof_reached){
      ret = send_next_job(file, client_sock, &eof_reached);
      if (ret == -1){
        perror("send()");
        return -1;
      }
    }
    return 0;
  } else if (req_type == 'T'){
    return 1;
  } else if (req_type == 'E'){
    return 2;
  } else if (req_type == 'Q'){
    return 5;
  }

  return -1;

}

/* Sends next job to server
* returns return to systemcall send()
*/
ssize_t send_next_job(FILE* file, int client_sock, int* eof_reached){
  char* message = read_next_job(file, eof_reached);
  printf("Sent: work type: %c, message: %s\n", message[0], (message + 2));
  ssize_t ret = send(client_sock, message, REPLY_MSG_SIZE, 0);
  free(message);
  return ret;
}
