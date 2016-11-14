#include <stdio.h>

int read_next_job(FILE* file, char* work_type, unsigned char* message_len, char* message);

/*
* Server!
*/
int main(int argc, char const *argv[]) {

  if (argc != 3) {
		fprintf(stderr, "Usage: %s <filename> <port>\n", argv[0]);
		return -1;
	}

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
    return -1;
  }

  //if client connected
  char work_type;
  unsigned char message_len;
  char message[256] = {0};

  if (read_next_job(file, &work_type, &message_len, message) == -1){
    //TODO send message to klient?
  }


  fclose(file);
  return 0;
}

/* Reads next job parameters from file into input parameters
*  returns -1 on error (f.eks end of file is reached), else 0
*/
int read_next_job(FILE* file, char* work_type, unsigned char* message_len, char* message){

  int bytes_read = 0;

  fread(work_type, sizeof(char), 1, file);
  printf("work_type: %c\n", *work_type);

  fread(message_len, sizeof(unsigned char), 1, file);
  printf("message_len: %u\n", *message_len);

  bytes_read = fread(message, sizeof(char), *message_len, file);
  printf("message: %s\n", message);

  if (bytes_read < 1){
    printf("Couldn't read next byte from file\n");
    return -1;
  }

  return 0;

}
