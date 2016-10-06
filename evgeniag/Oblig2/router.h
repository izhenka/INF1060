
#include <stdio.h>
#include <stdlib.h>

struct router {
  unsigned char id;
  unsigned char flag;
  char producer[256];
};

struct router* router_init(void);
void router_pretty_print(struct router* r);
void print_binary(char num);
char* check_flag_bit(char flag, int bit_pos);
int router_get_modify_number(struct router* r);
int bit_is_set(char x, int bit_pos);
char* str_boolean(int boolean);
void router_modify_flag(struct router* r, int bit_pos);
int router_increase_modify_number(struct router* r);
void router_set_producer(struct router* r, char* producer);
void router_set_id(struct router* r, unsigned char id);
