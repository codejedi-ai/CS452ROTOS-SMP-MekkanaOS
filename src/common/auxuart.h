#ifndef _AUXUART_H_
#define _AUXUART_H_ 1

#include <stdint.h>

/* BCM2835 AUX mini-UART — QEMU serial1 (ROTS network link wire). */

void auxuart_init(void);
void auxuart_putc(unsigned char c);
int  auxuart_getc_nb(void);
unsigned char auxuart_getc(void);

#endif /* _AUXUART_H_ */
