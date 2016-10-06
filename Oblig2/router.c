
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct router {
  unsigned char id;
  unsigned char flag;
  char producer[256];
};


void router_pretty_print(struct router* r);
void print_binary(char num);
char* check_flag_bit(char flag, int bit_pos);
int router_get_modify_number(struct router* r);
int bit_is_set(char x, int bit_pos);
char* str_boolean(int boolean);
void router_modify_flag(struct router* r, int bit_pos);
int router_increase_modify_number(struct router* r);
struct router* router_init(void);
void router_set_producer(struct router* r, char* producer);
void router_set_id(struct router* r, unsigned char id);
int router_deacrease_modify_number(struct router* r);
void router_pretty_print_flags(struct router* r);


struct router* router_init(void){
  return malloc(sizeof(struct router));
};

void router_pretty_print(struct router* r){

  printf("\nRouter%d: %s\tflags:", r->id, r->producer);
  print_binary(r->flag);
  router_pretty_print_flags(r);
  printf("Modify number: \t%d\n\n", router_get_modify_number(r));

};

void router_pretty_print_flags(struct router* r){
  /*
  0 Aktive
  1 Wireless
  2 5GHz
  3 Unused
  4:7 Modify number
  */

  printf("Active: \t%s\nWireless: \t%s\nSupports 5GHz: \t%s\nUnused: \t%s\n",
  check_flag_bit(r->flag, 0), check_flag_bit(r->flag, 1), check_flag_bit(r->flag, 2), check_flag_bit(r->flag, 3));
}

int bit_is_set(char x, int bit_pos){

  char mask = 1<<bit_pos;
  return ((x & mask) == mask);

}

int router_get_modify_number(struct router* r){
  char mask4_7 = 0xf0;
  return (r->flag & (mask4_7))>>4;
}

int router_increase_modify_number(struct router* r){
  char number = router_get_modify_number(r);
  if (number==15){
    printf("Router parameters can't be modified more then 15 times!");
    return 0;
  }

  char mask0_3 = 0x0f;
  number += 1;
  r->flag = (number<<4) | (r->flag & mask0_3);

  return 1;
}

int router_deacrease_modify_number(struct router* r){
  char number = router_get_modify_number(r);
  if (number==0){
    return 0;
  }

  char mask0_3 = 0x0f;
  number -= 1;
  r->flag = (number<<4) | (r->flag & mask0_3);

  return 1;
}

char* str_boolean(int boolean){
  return (boolean>0) ? "+": "-";
}

char* check_flag_bit(char flag, int bit_pos){
  return str_boolean(bit_is_set(flag, bit_pos));
}

void router_set_id(struct router* r, unsigned char id){
  r->id = id;
};

void router_set_flag(struct router* r, unsigned char flag){
  r->flag = flag;
};

void router_set_producer(struct router* r, char* producer){
  if (router_increase_modify_number(r)){
    memmove(r->producer, producer, strlen(producer) + 1);
    printf("Producer was succefully modified to %s\n", r->producer);
  }
};

void router_modify_flag(struct router* r, int bit_pos){
  if (router_increase_modify_number(r)){
    char mask = 1<<bit_pos;
    r->flag = r->flag ^ mask;
    printf("Parameter[%d] was succefully modified to %s\n", bit_pos, check_flag_bit(r->flag, bit_pos));
  }
};

void print_binary(char num){
  int pos = (sizeof(char) * 8) - 1;

  for (int i = 0; i < (int)(sizeof(char) * 8); i++) {
    char c = num & (1 << pos) ? '1' : '0';
    putchar(c);
    if (!((i + 1) % 8))
    putchar(' ');
    pos--;
  }
  putchar('\n');
}
