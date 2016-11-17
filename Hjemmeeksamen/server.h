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
