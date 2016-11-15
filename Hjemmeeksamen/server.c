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

int read_next_job(FILE* file, char* work_type, unsigned char* message_len, char* message);
int create_socket(int port);
void terminate(int signum);
int accept_connection(int server_sock);

int running = 1; //continue to accept connections

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
  printf("Socket created successfully, num: %d\n", sock);


  while (running){
    //accept connection
    printf("Waiting for connection...\n");
    int client_sock = accept_connection(sock);
    if (client_sock == -1){
      continue;
    }

    ssize_t ret;
    char buf[256] = { 0 };
  	ret = recv(client_sock, buf, sizeof(buf) - 1, 0);
    if (ret == 0) {
    	printf("Client disconnected.\n");
    }else if (ret == -1) {
  		perror("recv()");
  		close(client_sock);
  		continue;
  	} else {
  		printf("Message from client: %s\n", buf);
  	}


    char work_type;
    unsigned char message_len;
    char message[256];

    if (read_next_job(file, &work_type, &message_len, message) == -1){
      //TODO send message to klient?
      strcpy(message, "I'm tired, bye! :*\n");
      running = 0;
    }

    ret = send(client_sock, message, strlen(message), 0);
    if (ret == -1){
      perror("send()");
      close(client_sock);
      continue;
    }

    close(client_sock);
  }


  close(sock);
  fclose(file);
  return 0;
}

/* Reads next job parameters from file into input parameters
*  returns -1 on error (f.eks end of file is reached), else 0
*/
int read_next_job(FILE* file, char* work_type, unsigned char* message_len, char* message){

  int bytes_read = 0;
  printf("****** reading *****\n");

  fread(work_type, sizeof(char), 1, file);
  printf("work_type: %c\n", *work_type);

  fread(message_len, sizeof(unsigned char), 1, file);
  printf("message_len: %u\n", *message_len);

  memset(message, '\0', 256);
  bytes_read = fread(message, sizeof(char), *message_len, file);
  printf("message: %s\n", message);

  if (bytes_read < 1){
    printf("Couldn't read next byte from file\n");
    return -1;
  }

  printf("****** reading *****\n");
  return 0;
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
  printf("Socket is set!\n");

  //setting internet address
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  printf("Sockaddr_in is set %u!\n", server_addr.sin_addr.s_addr);

  //binding address to socket
  if (bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0){
    perror("bind()");
    close(sock);
    return -1;
  }
  printf("Address is binded!\n");

  //activate listening to connections
  if (listen(sock, SOMAXCONN) != 0) {
		perror("listen()");
		close(sock);
		return -1;
	}
  printf("Listening is on!\n");

  return sock;

}

/* SIGINT handler
* terminates connections acception
*/
void terminate(int signum)
{
	printf("\nTerminating server with signal: %d\n", signum);
	running = 0;
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
