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

int connect_to_server(char* ip, int port);

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		return -1;
	}

	int port = (int) strtol(argv[2], NULL, 10);
	if(port == 0){
		perror("Wrong port number");
		return -1;
	}

	int sock = connect_to_server(argv[1], port);
	if (sock == -1){
    exit(EXIT_FAILURE);
	}

	char msg[256] = "Hei, server!";
	ssize_t ret = send(sock, msg, strlen(msg), 0);
	if (ret == -1) {
		perror("send()");
		close(sock);
		exit(EXIT_FAILURE);
	}

	char buf[256] = { 0 };
	ret = recv(sock, buf, sizeof(buf) - 1, 0);
	if (ret == 0) {
		printf("Server disconnected.\n");
	} else if (ret == -1) {
		perror("recv()");
		close(sock);
		exit(EXIT_FAILURE);
	} else {
		printf("Message from server: %s\n", buf);
	}

	close(sock);
	return 0;

	/*

  pid_t pid = fork();
  if (pid == -1){
    perror("fork()");
    exit(EXIT_FAILURE);
  }

  if (pid == 0){ //barneprosess
    printf("Jeg er barn! Message: %s\n", argv[1]);
    return 2;
  }else{
    int status;
    wait(&status);
    printf("Child terminated with status %d\n", WEXITSTATUS(status));
  }

*/



	// int flags = MAP_SHARED | MAP_ANONYMOUS;
	// char *mem2 = malloc(512);
	// char *mem = mmap(NULL, 512, PROT_READ | PROT_WRITE, flags, -1, 0);
	// if (mem == MAP_FAILED) {
	// 	perror("mmap()");
	// 	exit(EXIT_FAILURE);
	// }
  //
	// printf("Yay: %p\n", mem);
  //
	// /* PID = Process ID */
	// pid_t pid = fork();
	// if (pid == -1) {
	// 	perror("fork()");
	// 	exit(EXIT_FAILURE);
	// }
  //
	// if (pid == 0) { /* child */
	// 	printf("Message: %s\n", mem);
	// 	sleep(1);
	// 	printf("Message: %s\n", mem);
	// 	return 42;
	// } else { /* parent */
	// 	sleep(1);
	// 	strcpy(mem, argv[1]);
	// 	//sprintf(mem2+strlen(argv[1]), "test");
	// 	//mem[strlen(argv[1])+strlen("test")] = 'a';
	// 	int status;
	// 	wait(&status);
	// 	printf("Child terminated with status %d\n", WEXITSTATUS(status));
	// 	printf("Parent message: %s\n", mem2);
	// }
  //
	// return 0;
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
  printf("Socket is set!\n");

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
  printf("Sockaddr_in is set %u!\n", server_addr.sin_addr.s_addr);


	printf("Connecting to %s:%d\n", ip, port);
	if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
		perror("connect()");
		close(sock);
		return -1;
	}
	printf("Connected!\n");

  return sock;
}
