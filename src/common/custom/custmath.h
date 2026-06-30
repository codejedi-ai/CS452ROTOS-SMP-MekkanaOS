#include "rpi.h"
#include "util.h"

uint64_t min(uint64_t a, uint64_t b);
uint64_t max(uint64_t a, uint64_t b);

/* Pack/unpack int64 values (byint: two 32-bit ints; octochar: 8 bytes). */
void int64_to_byint(uint64_t byint_charint, int *a, int *b);
uint64_t byint_to_int64(int a, int b);
uint64_t octochar_to_int64(char *str);
void int64_to_octochar(uint64_t charint, char *str);
