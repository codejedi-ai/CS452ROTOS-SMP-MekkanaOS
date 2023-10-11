#include "util.h"
#include "syscall.h"
int8_t is_Hex(char *switch_number);

char str_to_int(char *str);

char hexstr_to_int(char *str);

void strcmp(int* ret, char* s1, char* s2);
int strcmp_ret(char* s1, char* s2);
int stringconcat(char* dest, const char* src);
// this fiunction would
int parse_char_arr(char *arr, char **num, int num_size);
// return strings in place
void parsestring(char * str, int part, char *retarr, int size);

int strcpy(char *dest, int lenDes, char *src, int lenSrc);

void strflush(char* msg, uint8_t msglen);