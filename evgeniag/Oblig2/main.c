
#include <stdio.h>
#include <string.h>
#include "router.h"

void commando_loop(void);
void print_router_info(void);
void modify_flag(void);
void modify_producer(void);
struct router* get_router_from_user();
void write_to_file(char* filename);

struct router** g_map;
int g_num_routers;


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


  fread(&g_num_routers, sizeof(g_num_routers), 1, file);
  printf("num_routers: %d\n", g_num_routers);
  fgetc(file); //skipping '/n'


  g_map = calloc(sizeof(struct router*), g_num_routers);

  for(int i = 0; i < g_num_routers; i++) {

    struct router* r = router_init();

    fread(&(r->id), sizeof(r->id), 1, file);
    fread(&(r->flag), sizeof(r->flag), 1, file);
    //producer
    unsigned char length_producer;
    fread(&length_producer, sizeof(length_producer), 1, file);
    r->producer = malloc(sizeof(char)*(length_producer));
    fread(r->producer, sizeof(char), length_producer-1, file); //without '\0' in file

    g_map[r->id] = r;

    fgetc(file); //skipping '/n'
  };
  fclose(file);


  printf("g_map:\n");
  for(int i = 0; i < g_num_routers; i++) {
    router_pretty_print(g_map[i]);
  }


  commando_loop();

  write_to_file("result.dat");

  //TODO free(r);
  return 0;
}


void write_to_file(char* filename){

  FILE *file = fopen(filename, "w+");
  if (file == NULL) {
    fprintf(stderr, "Couldn't open file: %s\n", filename);
    return;
  }

  char new_line = '\n';
  fwrite(&g_num_routers, sizeof(int), 1, file);
  fwrite(&new_line, sizeof(char), 1, file);


  for(int i = 0; i < g_num_routers; i++) {

    struct router* r = g_map[i];

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
      case 3: modify_producer(); break;
      case 4: printf("4--\n"); break;
      case 5: printf("5--\n"); break;
      case 6: exit = 1; break;
      default: printf("Ugyldig kommado!\n");
    }


  }

}

void print_router_info(void){
  struct router* r = get_router_from_user();
  if (r != NULL){
      router_pretty_print(r);
  }
}

struct router* get_router_from_user(){
  printf("\nTast ruter id:");
  char res_s[256];
  fgets(res_s, 256, stdin);
  int res = (int) strtol(res_s, NULL, 10);

  if (res>g_num_routers){
    fprintf(stdout, "Maks id er %d\n", (g_num_routers-1));
    return NULL;
  }

  struct router* r = g_map[res];
  if (r == NULL){
    fprintf(stdout, "Finnes ikke ruter med id %d\n", res);
  }

  return r;

}

void modify_flag(void){
  struct router* r = get_router_from_user();
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

void modify_producer(void){
  struct router* r = get_router_from_user();
  if (r == NULL){
      return;
  }

  printf("\nSkriv inn ny produsent\\model:");
  char res_s[256];
  fgets(res_s, 256, stdin);
  printf("\nTakk for det og fuck you");


}
