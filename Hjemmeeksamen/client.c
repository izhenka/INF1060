#include "client.h"

//GLOBAL VARIABLES
int g_sock; //client socket
int user_terminate = 0;
int terminate_request_server = 0;
int fds[2][2];


int main(int argc, char *argv[]){

	int port = get_port_number(argc, argv);
	if (!port){
		exit(EXIT_FAILURE);
	}

	//setting pipes
	 if (pipe(fds[0]) == -1) {
		 perror("pipe()");
		 exit(EXIT_FAILURE);
	 }
	 if (pipe(fds[1]) == -1) {
		 perror("pipe()");
		 exit(EXIT_FAILURE);
	 }

	 pid_t pid1, pid2;
	 int quit = 0;

	 //forking 2 children
	 switch(pid1 = fork()){
		case -1: 		perror("fork()");
		 						exit(EXIT_FAILURE);

		 case 0:		//CHILD 1
		 						set_sig_handler(&terminate_ch1, SIGINT);
								while (!user_terminate && !quit) {
									quit = read_from_parent(1);
								}
								printf("Child 1 terminated x_x.\n");
								break;
		default:

								switch(pid2 = fork()){
									case -1: 		perror("fork()");
															send_quit_to_childs();
															wait(&pid1);
															exit(EXIT_FAILURE);

									case 0:		//CHILD 2
														set_sig_handler(&terminate_ch2, SIGINT);
														while (!user_terminate && !quit) {
															quit = read_from_parent(2);
														}
														printf("Child 2 terminated x_x.\n");
														break;

									default:	//PARENT

														set_sig_handler(&terminate, SIGINT);

														g_sock = connect_to_server(argv[1], port);
														if (g_sock == -1){
															send_quit_to_childs();
															wait(&pid1); wait(&pid2);
													    exit(EXIT_FAILURE);
														}

														int termination_type = commando_loop();
														wait(&pid1); wait(&pid2);
														send_term_confirmation(termination_type);
														close(g_sock);
		}
	 }

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

void terminate_ch1(int signum){
	printf("\nChild 1: terminating(signal %d).\n", signum);
	user_terminate = 1;
}

void terminate_ch2(int signum){
	printf("\nChild 2: terminating(signal %d).\n", signum);
	user_terminate = 1;
}



/* Sends confirmation of termination to server
*  according to termination conditions
*/
void send_term_confirmation(int termination_type){
	int conf_sent = 0;
	switch (termination_type) {
		case 1: 	conf_sent = send_exit(); break;
		case 2: 	conf_sent = send_exit_after_req(); break;
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

/*  Provides interaction with user
*		returns termination type:
*		1 - normal
*		2 - after request from server_addr
*	 -1 - termination on error
*/
int commando_loop(){

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
			case 4: send_exit(); send_quit_to_childs(); exit = 1; break;
      default: printf("Ugyldig kommado!\n");
    }
  }

	return exit;
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
		printf("Server is done with jobs and asked to terminate.\n");
		send_quit_to_childs();
		*exit = 2;
		return;
	}

	//Got message
	printf("Message from server: %s\n", msg);
	send_to_child(child_num(work_type), msg);

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
			printf("Server is done with jobs and asked to terminate.\n");
			send_quit_to_childs();
			*exit = 2;
			return;
		}
		//Got message
		printf("Message from server: %s\n", msg);
		send_to_child(child_num(work_type), msg);

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
			printf("Server is done with jobs and asked to terminate.\n");
			send_quit_to_childs();
			*exit = 2;
			return;
		}
		//Got message
		printf("Message from server: %s\n", msg);
		send_to_child(child_num(work_type), msg);

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

/* Gets port number from arguments
* returns
*		port number if check is ok
*		0 on error
*/
int get_port_number(int argc, char *argv[]){
	//arguments checking
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		return 0;
	}

	int port = (int) strtol(argv[2], NULL, 10);
	if(port == 0){
		perror("Wrong port number");
		return 0;
	}

	return port;
}

/* Sends message msg
*  to child nummer child_num
*/
void send_to_child(int child_num, char* msg){
	close(fds[child_num-1][0]);
	write(fds[child_num-1][1], msg, 256);
}

/* Reads and prints message from parent
* to said child number
* returns 1 if parents asks to quit
* 				0 if not
*/
int read_from_parent(int child_num){
	char readbuf[256] = {0};
	close(fds[child_num-1][1]);
	read(fds[child_num-1][0], readbuf,sizeof(readbuf));
	if ((user_terminate) || (readbuf[0] == 'Q')){
		return 1;
	}
	if (child_num == 1){
		fprintf(stdout, "Child %d: %s\n", child_num, readbuf);
	} else{
		fprintf(stderr, "Child %d: %s\n", child_num, readbuf);
	}

	return 0;
}

/* Returns child number
* from said work type
*/
int child_num(char work_type){
	return ( work_type == 'O' ? 1 : 2);
}

/* Sets signal hadler
* Input: function pointer to handler
*					signal nummer
*/
void set_sig_handler(void (*handler) (int), int sig){
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handler;

	if (sigaction(sig, &sa, NULL) != 0) {
		perror("sigaction()");
		exit(EXIT_FAILURE);
	}
}

/* Sends quit-message to childs
*/
void send_quit_to_childs(){
	char msg[1] = {'Q'};
	send_to_child(1, msg);
	send_to_child(2, msg);
}
