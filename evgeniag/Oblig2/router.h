
#include <stdio.h>
#include <stdlib.h>

struct router {
  unsigned char id;
  char flag;
  char* producer;
};

struct router* router_init(void);
void router_pretty_print(struct router* r);
void router_set_id(struct router* r, unsigned char id);
void router_set_flag(struct router* r, char flag);
void router_set_producer(struct router* r, char* producer);
