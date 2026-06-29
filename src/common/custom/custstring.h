#ifndef _custstring_h_
#define _custstring_h_ 1

#include "util.h"
#include "syscall.h"

/* Freestanding string helpers. Names are deliberately non-libc to make it
   obvious these are our own implementations -- there is no libc linked into
   the kernel image. */

int8_t  is_empty(char *str);
int64_t atoi_64(char *str);

/* Returns 1 if s1 and s2 are byte-equal (including terminator), 0 otherwise.
   Note: inverted relative to libc strcmp -- this is "equals", not "compare". */
int     strcmp_ret(char *s1, char *s2);

/* Splits arr in place on space characters; writes pointer-to-each-token into
   num[]. Returns the number of tokens written. */
int     parse_char_arr(char *arr, char **num, int num_size);

/* Bounded copy. Stops at NUL or when either lenDes/lenSrc is reached.
   Always writes a terminator. Returns bytes copied (excluding terminator). */
int     cust_strcpy(char *dest, int lenDes, char *src, int lenSrc);

/* Appends src onto dest (which must already be NUL-terminated). Returns the
   new length of dest (excluding terminator). */
int     strcat_cust(char *dest, const char *src);

#endif /* custstring.h */
