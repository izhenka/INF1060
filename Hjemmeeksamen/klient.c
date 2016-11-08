#include <stdio.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
		return 0;
	}

  pid_t pid = fork();
  if (pid == -1){
    perror("fork()");
    exit(EXIT_FAILURE);
  }

  if (pid == 0){ //barneprosess
    printf("Message: %s\n", arg[1]);
    return 42;
  }else{
    int status;
    wait(&status);
    printf("Child terminated with status %s\n", WEXITSTATUS(status));
  }

  return 0;



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
