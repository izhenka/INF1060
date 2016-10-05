
#include <stdio.h>
#include "router.h"

struct router** g_map;

int main(int argc, char const *argv[]) {

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  FILE *fil = fopen(argv[1], "r");
  if (fil == NULL) {
    fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
    return -1;
  }

  int num_routers;

  fread(&num_routers, sizeof(num_routers), 1, fil);
  printf("num_routers: %d\n", num_routers);
  fgetc(fil); //skipping '/n'


  g_map = calloc(sizeof(struct router*), num_routers);

  for(int i = 0; i < num_routers; i++) {

    struct router* r = router_init();

    fread(&(r->id), sizeof(r->id), 1, fil);
    fread(&(r->flag), sizeof(r->flag), 1, fil);
    //producer
    unsigned char length_producer;
    fread(&length_producer, sizeof(length_producer), 1, fil);
    r->producer = malloc(sizeof(char)*(length_producer-1));
    fread(r->producer, sizeof(char), length_producer-1, fil);

    g_map[r->id] = r;

    fgetc(fil); //skipping '/n'
  };

  printf("g_map:\n");
  for(int i = 0; i < num_routers; i++) {
    router_pretty_print(g_map[i]);
  }

  //TODO free(r);

  fclose(fil);
  return 0;
}
