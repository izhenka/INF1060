
#include <stdio.h>
#include <string.h>
#include "router.h"

void commando_loop(void);
void print_router_info(void);
void modify_flag(void);
void modify_producer(struct router* r);
struct router* get_router_from_user(int* id);
void write_to_file(char* filename);
void strip_newline(char *s);
void check_newline(FILE *fp);
void new_router(void);
void delete_router(void);
void print_all(void);
void free_all(void);
void read_data_from_file(FILE *file);
int get_num_routers(void);
struct router* find_router(int* id);

#define MAX_ROUTERS 256
struct router* g_map[MAX_ROUTERS];


int main(int argc, char const *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
    return -1;
  }

  read_data_from_file(file);
  fclose(file);

  commando_loop();
  write_to_file("result.dat");
  free_all();

  return 0;
}


void read_data_from_file(FILE *file){

  int num_routers;
  fread(&num_routers, sizeof(int), 1, file);
  printf("num_routers: %d\n", num_routers);
  check_newline(file);

  for(int i = 0; i < num_routers; i++) {

    struct router* r = router_init();

    fread(&(r->id), sizeof(r->id), 1, file);
    fread(&(r->flag), sizeof(r->flag), 1, file);
    //producer
    unsigned char length_producer;
    fread(&length_producer, sizeof(length_producer), 1, file);
    char producer[length_producer];
    memset(producer, 0, length_producer);
    fread(producer, sizeof(char), length_producer-1, file);
    strcpy(r->producer, producer);
    //strip_newline(r->producer);

    g_map[r->id] = r;

    check_newline(file);
  };

}

void write_to_file(char* filename){

  FILE *file = fopen(filename, "w+");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open file: %s\n", filename);
    return;
  }

  char new_line = '\n';
  int num_routers = get_num_routers();
  fwrite(&num_routers, sizeof(int), 1, file);
  fwrite(&new_line, sizeof(char), 1, file);

  for(int i = 0; i < MAX_ROUTERS; i++) {

    struct router* r = g_map[i];
    if (r == NULL){
      continue;
    }

    fwrite(&(r->id), sizeof(r->id), 1, file);
    fwrite(&(r->flag), sizeof(r->flag), 1, file);
    //producer
    unsigned char length_producer = strlen(r->producer)+1;
    fwrite(&length_producer, sizeof(length_producer), 1, file);
    fwrite(r->producer, sizeof(char), length_producer-1, file); //cutting '\0' in file
    fwrite(&new_line, sizeof(char), 1, file);

  };

  fclose(file);
}

void commando_loop(void){

  int exit = 0;
  while(!exit){

    printf("\nTast kommando:\n");
    printf("\t1. Printe info om ruter gitt en ID\n");
    printf("\t2. Endre flagg for en ID\n");
    printf("\t3. Endre produsent/modell for en ID\n");
    printf("\t4. Legge inn en ny ruter (bruker putter inn ID og annen data)\n");
    printf("\t5. Slette en ruter fra databasen\n");
    printf("\t6. Avslutte programmet\n");

    char res_s[256];
    fgets(res_s, 256, stdin);
    int res = (int) strtol(res_s, NULL, 10);

    switch (res) {
      case 1: print_router_info(); break;
      case 2: modify_flag(); break;
      case 3: modify_producer(NULL); break;
      case 4: new_router(); break;
      case 5: delete_router(); break;
      case 6: exit = 1; break;
      case 7: print_all(); break;
      default: printf("Ugyldig kommado!\n");
    }
  }
}

void new_router(void){
  printf("Creating new router.\n");
  int id;
  struct router* r = get_router_from_user(&id);
  if (r != NULL){
    printf("Router with id %d already exists!\n", r->id);
    return;
  }

  r = router_init();
  router_set_id(r, id);
  g_map[id] = r;
  modify_producer(r);




}

void delete_router(void){
  int id;
  struct router* r = get_router_from_user(&id);
  if (r != NULL){
    unsigned char id = r->id;
    free(g_map[id]);
    g_map[id] = NULL;
    printf("Router %d is deleted!\n", id);
  }
}

void print_router_info(void){
  int id;
  struct router* r = get_router_from_user(&id);
  if (r != NULL){
      router_pretty_print(r);
  }
}

void print_all(void){
  printf("g_map:\n");
  for(int i = 0; i < MAX_ROUTERS; i++) {
    if (g_map[i] == NULL){
      continue;
    }
    router_pretty_print(g_map[i]);
  }
}

void free_all(void){
  for(int i = 0; i < MAX_ROUTERS; i++) {
    if (g_map[i] == NULL){
      continue;
    }
    free(g_map[i]);
  }
}

int get_num_routers(void){
  int res = 0;
  for(int i = 0; i < MAX_ROUTERS; i++) {
    if (g_map[i] != NULL){
      res+=1;
    }
  }
  return res;
}

struct router* get_router_from_user(int* id){
  printf("\nTast ruter id:");
  char res_s[256];
  fgets(res_s, 256, stdin);
  int res = (int) strtol(res_s, NULL, 10);

  if (res>MAX_ROUTERS){
    fprintf(stdout, "Maks id er %d\n", (MAX_ROUTERS-1));
    return NULL;
  }

  struct router* r = g_map[res];
  if (r == NULL){
    fprintf(stdout, "Finnes ikke ruter med id %d\n", res);
  }

  *id = res;
  return r;

}

struct router* find_router(int* id){
  printf("\nTast ruter id:");
  char res_s[256];
  fgets(res_s, 256, stdin);
  int res = (int) strtol(res_s, NULL, 10);

  if (res>MAX_ROUTERS){
    fprintf(stdout, "Maks id er %d\n", (MAX_ROUTERS-1));
    return NULL;
  }

  *id = res;
  return g_map[res];

}



void modify_flag(void){
  int id;
  struct router* r = get_router_from_user(&id);
  if (r == NULL){
      return;
  }
  router_pretty_print(r);

  printf("\nVelg parameter:\n");
  printf("\t1.Active\n");
  printf("\t2.Wireless\n");
  printf("\t3.Supports 5GHz\n");
  printf("\t4.Unused\n");
  printf("\t5.--exit\n");

  char res_s[256];
  fgets(res_s, 256, stdin);
  int res = (int) strtol(res_s, NULL, 10);

  if (res > 0 && res < 5){
    router_modify_flag(r, res-1);
  } else if (res == 5){
    return;
  } else {
    printf("Ugyldig kommado!\n");
  }

}

void modify_producer(struct router* r){

  if (r == NULL){
    int id;
    struct router* r = get_router_from_user(&id);
    if (r == NULL){
      return;
    }
  };

  printf("\nSkriv inn ny produsent\\model:");
  char res_s[256];
  fgets(res_s, 256, stdin);
  strip_newline(res_s);
  router_set_producer(r, res_s);

}

void strip_newline(char* s){
	char* ptr = strchr(s, '\n');
	if (ptr != NULL)
		*ptr = '\0';
}

void check_newline(FILE *fp){
	/* newline = '\n' = 0xa = 10 */
	int c = fgetc(fp);
	if (c != '\n') {
		fprintf(stderr, "Expected newline, got 0x%x (%d)\n", c, c);
		fclose(fp);
		exit(EXIT_FAILURE);
	}
}
