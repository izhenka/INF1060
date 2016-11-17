#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

void get_job(int* exit);
void get_x_jobs(int* exit);
void get_all_jobs(int* exit);
void terminate(int signum);
int connect_to_server(char* ip, int port);
int commando_loop();
int receive_from_serv(char* msg, char* work_type);
int send_req(char c, int num);
int send_exit();
int send_error_exit();
int send_exit_after_req();
void send_term_confirmation(int termination_type);

#define INCOM_MSG_SIZE 258
#define REQ_MSG_SIZE 2
