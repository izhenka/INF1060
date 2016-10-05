#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int stringsum(char* s);
int distance_between(char* s, char c);
char* string_between(char* s, char c);
char** split(char* s);
void stringsum2(char* s, int* res);


int stringsum(char* s){
  int result = 0;
  for (int i = 0; i<strlen(s); i++){
    if (s[i]>=65 && s[i]<=90)
    result += (s[i] - 65 + 1);
    else if (s[i]>=97 && s[i]<=122)
    result += (s[i] - 65 - 32 + 1);
    else
    return -1;
    //printf("char: %c, dec %d\n", s[i], s[i]);
  }

  return result;
}

int distance_between(char* s, char c){

  char* pos1 = index(s, (int)c);
  if (pos1 == NULL)
  return -1;

  char* pos2 = index((pos1 + 1), (int)c);
  //printf("pos1: %s, pos2 %s\n", pos1, pos2);
  if (pos2 == NULL)
  return -1;

  return (pos2-pos1);
}

char* string_between(char* s, char c){

  char* result;

  char* pos1 = index(s, (int)c);
  if (pos1 == NULL)
  return NULL;

  int len = distance_between(s, c) - 1;
  if (len < 0)
    return NULL;

  result = malloc(len + 1);
  strncpy(result, (pos1 + 1), len);
  result[len] = '\0';
  return result;

}

char** split(char* s){

  //figures out how many words we have
  char* pos = index(s, (int)' ');
  int words_count = 1;
  while (pos != NULL) {
    pos = index(pos + 1, (int)' ');
    words_count++;
  }


  //filling the array
  char** result = calloc(words_count + 1, sizeof(char *));

  char* tmp = malloc(2+ strlen(s));
  strcat(tmp, " ");
  strcat(tmp, s);
  strcat(tmp, " ");

  int length = 0;
  for (int i = 0; i< words_count; i++){
    result[i] = string_between((tmp + length), (int)' ');;
    length+= strlen(result[i]) + 1;
  }
  result[words_count] = NULL;

  free(tmp);
  return result;

}

void stringsum2(char* s, int* res){
  *res = stringsum(s);
}
