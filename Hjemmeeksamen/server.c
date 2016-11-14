



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
  int str_length = 0;
  int bytes_read = fread(&str_length, sizeof(int), 1, file);
  if (bytes_read < 1){
    printf("Couldn't read next byte from file\n");
    return -1; //TODO send message to klient?
  }
  printf("str_length: %d\n", str_length);



  fclose(file);
  return 0;
}
