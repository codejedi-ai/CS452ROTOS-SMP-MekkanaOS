#include "util.h"
#include "syscall.h"
#include <stdlib.h>
int8_t is_empty(char *str);
int8_t is_hex(char *switch_number);

int64_t atoi_64(char *str);

int64_t str_to_hex(char *str);

void strcmp_inpace(int* ret, char* s1, char* s2);
// strcmp_ret — return non-zero iff s1 and s2 match.
//   case_agnostic = 0: byte-for-byte compare.
//   case_agnostic = 1: fold ASCII A-Z / a-z before comparing each byte.
int strcmp_ret(char* s1, char* s2, int case_agnostic);
// this fiunction would
int parse_char_arr(char *arr, char **num, int num_size);


int cust_strcpy(char *dest, int lenDes, char *src, int lenSrc);

void strflush(char* msg, uint8_t msglen);
int strcat_cust(char* dest, const char* src);