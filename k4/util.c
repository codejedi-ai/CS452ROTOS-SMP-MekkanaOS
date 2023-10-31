#include "util.h"
#include "rpi.h"


// ascii digit to integer
int a2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

// unsigned int to ascii string
void ui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;

	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

// signed int to ascii string
void i2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	ui2a( num, 10, bf );
}

// define our own memset to avoid SIMD instructions emitted from the compiler
void *memset(void *s, int c, size_t n) {
  for (char* it = (char*)s; n > 0; --n) *it++ = c;
  return s;
}

// define our own memcpy to avoid SIMD instructions emitted from the compiler
void* memcpy(void* restrict dest, const void* restrict src, size_t n) {
    char* sit = (char*)src;
    char* cdest = (char*)dest;
    for (size_t i = 0; i < n; ++i) *(cdest++) = *(sit++);
    return dest;
}

void print_time(long unsigned int time, unsigned int row, unsigned int column, size_t line) {
  // This functions prints the time in int line
  unsigned int ms = time % 10;
  time = time / 10;
  unsigned int s = time % 60;
  time = time / 60;
  unsigned int m = time % 60;
  time = time / 60;
  unsigned int h = time % 100;
  
  uart_printf(line, "\033[%u;%uH",row, column);
  uart_printf(line, "time %u%u%u:%u%u:%u%u.%u", h/100, (h/10)%10, h%10, m/10, m%10, s/10, s%10, ms);
  
  if (s % 4 == 0) uart_printf(line, " -");
  else if (s % 4 == 1) uart_printf(line, " \\");
  else if (s % 4 == 2) uart_printf(line, " |");
  else if (s % 4 == 3) uart_printf(line, " /");
  
  return;
}

// print_mstime(ttemp, -1, CONSOLE);

void print_mstime(long unsigned int time, unsigned int id, size_t line, int offset) {
  int row = 5;
  int col = 65;
  int rstep = 4;
  int cstep = 11;
  uart_printf(line, "\033[33m");
  
  time = time / 100;
  
  uart_printf(line, "\033[%u;%uH", row + (id % 2) * rstep + offset, col + (id / 2) * cstep);
  
  if (time > 99999) {
    uart_printf(line, "9999.9");
  }
  else {
    uart_printf(line, "%u%u%u%u.%u", time / 10000, (time / 1000) % 10, (time / 100) % 10, (time / 10) % 10, time % 10);
  }
  
  uart_printf(line, "\033[0m");

  return;
}

void print_tspeed(int arr[], size_t alen, size_t line) {
  int row = 14;
  int col = 13;
  int cstep = 16;
  int i = 1;
  for (uint32_t j = 0; j < alen; j++) {
    if (arr[j] != 0) {
      int tv = arr[j];
      uart_printf(line, "\033[%u;%uH", row + (i % 3), col + (i / 3) * cstep);
      if (tv == -1) {
        uart_printf(line, "\033[31m%u%u \033[33m R", j/10, j%10);
      }
      else {
        uart_printf(line, "\033[31m%u%u \033[33m%u%u", j/10, j%10, tv/10, tv%10);
      }
      i++;
    }
  }
  while (i < 9) {
  
    uart_printf(line, "\033[%u;%uH", row + (i % 3), col + (i / 3) * cstep);
    uart_printf(line, "     ");
    i++;
  
  }
  uart_printf(line, "\033[0m");
  return;
}

void print_sensors(int arr[], size_t alen, unsigned int column, unsigned int row, size_t line) {
  uart_printf(line, "\033[33m");
  for (uint32_t i = 0; i < alen; i++) {
    if (arr[i] != 0) {
      uart_printf(line, "\033[%u;%uH",row + i, column);
      char ch = 'A';
      uart_putc(line, ch + (arr[i] / 17));
      uart_printf(line, "%d", arr[i]%17);
      uart_putc(line,' ');
    }
  }
  uart_printf(line, "\033[0m");
  
  return;
}


void sensor_push(int sen, int arr[], size_t alen) {
  int temp;
  for (uint32_t i = 0; i < alen; ++i) {
    temp = arr[i];
    arr[i] = sen;
    sen = temp;
  }
  return;
}

void print_switch(int s, char c, size_t line) {
  int row = 4;
  int col = 10;
  int step = 9;
  int row2 = 10;
  int col2 = 19;
  int step2 = 10;
  
  if (c == 'C') {
    uart_printf(line, "\033[36m");
  }
  else {
    uart_printf(line, "\033[32m");
  }
  
  
  if (s > 100) {
    s = s - 153;
    uart_printf(line, "\033[%u;%uH",row2 + (s / 2), col2 + step2 * (s % 2));
    uart_putc(line, c);
  
  }
  else {
    s = s - 1;
    uart_printf(line, "\033[%u;%uH",row + (s / 4), col + step * (s % 4));
    uart_putc(line, c);
  
  }
  uart_printf(line, "\033[0m");


}



