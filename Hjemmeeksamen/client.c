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
void commando_loop(int* termination_type);
int receive_from_serv(char* msg, char* work_type);
int send_req(char c, int num);
int send_exit();
int send_error_exit();
int send_exit_after_req();
void send_term_confirmation(int termination_type);

//GLOBAL VARIABLES
int g_sock; //client socket
int user_terminate = 0;

#define INCOM_MSG_SIZE 258
#define REQ_MSG_SIZE 2


//---*****------MAIN -----------****----------
int main(int argc, char *argv[]){

	//arguments checking
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		return -1;
	}

	int port = (int) strtol(argv[2], NULL, 10);
	if(port == 0){
		perror("Wrong port number");
		return -1;
	}

	//Cntrl + c handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = terminate;

	if (sigaction(SIGINT, &sa, NULL) != 0) {
		perror("sigaction()");
		exit(EXIT_FAILURE);
	}

	//forking 2 childs
	pid_t pid1 = fork();
	if (pid1 == -1) {
		perror("fork()");
		exit(EXIT_FAILURE);
	}

	if (pid1 == 0) { /* child 1 */
		fprintf(stderr, "Hei, jeg er barn 1!\n");
		return 0;

	} else {		/* parent */
		fprintf(stderr, "Jeg er parent. Fikk barn %d!\n", pid1);
	}

	return 0; //child-parent test

	//server communication
	g_sock = connect_to_server(argv[1], port);
	if (g_sock == -1){
    exit(EXIT_FAILURE);
	}

	int termination_type = 0;
	commando_loop(&termination_type);
	send_term_confirmation(termination_type);
	close(g_sock);

	return 0;
}


//OTHER FUNCTIONS

/* SIGINT handler
* terminates connections acception
*/
void terminate(int signum){
	printf("\nTerminating client (signal %d).\n", signum);
	user_terminate = 1;
}

/* Sends confirmation of termination to server
*  according to termination conditions
*/
void send_term_confirmation(int termination_type){
	int conf_sent = 0;
	switch (termination_type) {
		case 1: 	conf_sent = send_exit(); break;
		case 2: 	conf_sent = send_exit_after_req();
							printf("Server is done with jobs and asked to terminate.\n");
							break;
		case -1: 	conf_sent = send_error_exit(); break;
		default: 	conf_sent = send_error_exit();
	}
	printf("Confirmation of termination %s\n", conf_sent ? "has been sent successfully!" : "hasn't been sent.");
}

/* Establishes connection to server
* returns socket index
* returns -1 on error
*/
int connect_to_server(char* ip, int port){

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
	int res = inet_pton(AF_INET, ip, &server_addr.sin_addr.s_addr);
  if (res != 1) {
    if (res == 0) {
      fprintf(stderr, "Invalid IP address: %s\n", ip);
    } else {
      perror("inet_pton()");
    }
    close(sock);
    return -1;
  }
  //printf("Sockaddr_in is set %u!\n", server_addr.sin_addr.s_addr);


	printf("Connecting to %s:%d\n", ip, port);
	if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
		perror("connect()");
		close(sock);
		return -1;
	}
	printf("Connected!\n");

  return sock;
}

void commando_loop(int* termination_type){

  int exit = 0;
  while(!exit){
    printf("\nTast kommando:\n");
    printf("\t1. Hent en jobb fra serveren\n");
    printf("\t2. Hent X antall jobber fra serveren\n");
    printf("\t3. Hent alle jobber fra serveren\n");
    printf("\t4. Avslutte programmet\n");

    char res_s[256];
    fgets(res_s, 256, stdin);
    int res = (int) strtol(res_s, NULL, 10);

		if (user_terminate){
			exit = -1;
			break;
		}

    switch (res) {
      case 1: get_job(&exit); break;
      case 2: get_x_jobs(&exit); break;
      case 3: get_all_jobs(&exit); break;
			case 4: send_exit(); exit = 1; break;
      default: printf("Ugyldig kommado!\n");
    }


  }

	*termination_type = exit;
}

/* Gets new job from server
* 	parameter: int* exit is set to
* 		-1 when programm should be terminated because of error/server disconnect
* 		2 on request to terminate from server
*/
void get_job(int* exit){

	if (!send_req('G', 0)){
		printf("Failed to send request to server.\n");
		return;
	}

	char msg[256] = { 0 };
	char work_type;
	int res = receive_from_serv(msg, &work_type);
	if (res == -1){
		*exit = -1;
		return;
	} else if (res == 2) {
		*exit = 2;
		return;
	}

	//Got message
	printf("Message from server: %s\n", msg);

	if (user_terminate){
		*exit = -1;
		return;
	}


}

/* Gets X new jobs from server
* 	parameter: int* exit is set to
* 		-1 when programm should be terminated because of error/server disconnect
* 		2 on request to terminate from server
*/
void get_x_jobs(int* exit){

	printf("\nAntall jobber [1..255]:");
  char number_jobs_s[256];
  fgets(number_jobs_s, 256, stdin);
  int number_jobs = (int) strtol(number_jobs_s, NULL, 10);

  if (number_jobs<1 || number_jobs>255){
    fprintf(stdout, "Feil antall\n");
    return;
  }

	//sending
	if (!send_req('N', number_jobs)){
		printf("Failed to send request to server.\n");
		return;
	}

	//receiving
	char msg[256] = { 0 };
	char work_type;
	for (int i = 0; i<number_jobs; i++){
		int res = receive_from_serv(msg, &work_type);
		if (res == -1){
			*exit = -1;
			return;
		} else if (res == 2) {
			*exit = 2;
			return;
		}
		//Got message
		printf("Message from server: %s\n", msg);

		if (user_terminate){
			*exit = -1;
			return;
		}
	}

}

/* Gets all jobs left from server
* 	parameter: int* exit is set to
* 		-1 when programm should be terminated because of error/server disconnect
* 		2 on request to terminate from server
*/
void get_all_jobs(int* exit){
	//sending
	if (!send_req('A', 0)){
		printf("Failed to send request to server.\n");
		return;
	}

	//receiving
	char msg[256] = { 0 };
	char work_type;
	for (;;){
		//printf("receive_from_serv ...\n");
		int res = receive_from_serv(msg, &work_type);
		if (res == -1){
			*exit = -1;
			return;
		} else if (res == 2) {
			*exit = 2;
			return;
		}
		//Got message
		printf("Message from server: %s\n", msg);

		if (user_terminate){
			*exit = -1;
			return;
		}

	}
}

/* Reads next message from server:
* returns
	-1 on error,
	1 job is get
	2 server asks to terminate
*/
int receive_from_serv(char* msg, char* work_type){

	memset(msg, 0, 256);
	char buf[INCOM_MSG_SIZE] ={0};

	ssize_t ret = recv(g_sock, buf, INCOM_MSG_SIZE, 0);
	if (ret == 0) {
		printf("Server disconnected.\n");
		return -1;
	} else if (ret == -1) {
		perror("recv()");
		return -1;
	} else {

		// printf("****test*** Message from server:\n");
		// printf("****test*** buf[0]: %c\n", buf[0]);
		// printf("****test*** buf[1]: %d\n", buf[1]);
		// printf("****test*** buf[2]: %s\n", (buf + 2));

		if (buf[0] == 'Q'){
			return 2; //server has no more jobs
		}

		*work_type = buf[0];
		memmove(msg, (buf + 2), strlen(buf)); //already with /0 at the end
		return 1; //job is get
	}
}

/* Sends request to server:
* Protocol
* each message of 2 bytes: char and unsigned char
* 'G'0 	- get 1 job
* 'A'0 	- get all jobs
* 'N'x - get x jobs
* 'T'0 	- client terminates normally
* 'E'0	- client terminates with error
*
* returns 0 on error, 1 on success
*/
int send_req(char c, int num){
	char msg[REQ_MSG_SIZE] = {0};
	msg[0] = c;
	msg[1] = (unsigned char) num;
	ssize_t ret = send(g_sock, msg, REQ_MSG_SIZE, 0);
	if (ret == -1) {
		perror("send()");
		return 0;
	}
	return 1;
}

/* Sends message of normal termination to server
* returns 0 on error, 1 on success
*/
int send_exit(){
	return send_req('T', 0);
}

/* Sends message of error termination to server
* returns 0 on error, 1 on success
*/
int send_error_exit(){
	return send_req('E', 0);
}

/* Sends message of termination after request from server
* returns 0 on error, 1 on success
*/
int send_exit_after_req(){
	return send_req('Q', 0);
}
