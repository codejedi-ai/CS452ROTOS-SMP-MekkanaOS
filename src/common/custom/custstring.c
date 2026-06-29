#include "custstring.h"
#include "rpi.h"
#include "util.h"

int8_t is_empty(char *str) {
  return (*str == '\0');
}

int64_t atoi_64(char *str) {
  /* Leading minus is consumed but the magnitude is returned unsigned and
     cast to int64_t. Pre-existing behavior; preserved on consolidation. */
  if (str[0] == '-') str++;
  uint64_t ret = 0;
  while (*str != '\0') {
    ret = 10 * ret;
    ret += a2d(*str);
    str++;
  }
  return (int64_t)ret;
}

int strcmp_ret(char *s1, char *s2) {
  while (*s1 && *s2) {
    if (*s1 != *s2) return 0;
    s1++;
    s2++;
  }
  return (*s1 == *s2);
}

int strcat_cust(char *dest, const char *src) {
  int newsz = 0;
  while (*dest) { dest++; newsz++; }
  while (*src)  { *dest++ = *src++; newsz++; }
  *dest = '\0';
  return newsz;
}

int parse_char_arr(char *arr, char **num, int num_size) {
  int i = 1;
  num[0] = arr;
  char *ptr = arr;
  while (*ptr != '\0') {
    if (*ptr == ' ') {
      *ptr = 0;
      num[i] = ptr + 1;
      i++;
      if (i >= num_size) return i;
    }
    ptr++;
  }
  return i;
}

int cust_strcpy(char *dest, int lenDes, char *src, int lenSrc) {
  int i = 0;
  while (*src) {
    if (i >= lenSrc) break;
    if (i >= lenDes) break;
    *dest++ = *src++;
    i++;
  }
  *dest = 0;
  return i;
}
