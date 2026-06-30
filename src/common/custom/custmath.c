#include "custmath.h"

uint64_t min(uint64_t a, uint64_t b)
{
	if (a < b)
		return a;
	return b;
}

uint64_t max(uint64_t a, uint64_t b)
{
	if (a > b)
		return a;
	return b;
}

void int64_to_byint(uint64_t charint, int *a, int *b)
{
	*a = charint >> 32;
	*b = charint & 0xFFFFFFFF;
}

uint64_t byint_to_int64(int a, int b)
{
	uint64_t ret = (uint32_t)a;
	ret <<= 32;
	ret |= (uint32_t)b;
	return ret;
}

uint64_t octochar_to_int64(char *str)
{
	return *(uint64_t *)str;
}

void int64_to_octochar(uint64_t charint, char *str)
{
	*(uint64_t *)str = charint;
}
