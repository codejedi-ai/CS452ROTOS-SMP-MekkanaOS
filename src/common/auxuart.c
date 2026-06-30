#include "auxuart.h"
#include "config.h"

#define AUX_ENABLES     (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x04))
#define AUX_MU_IO_REG   (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x40))
#define AUX_MU_IER_REG  (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x44))
#define AUX_MU_IIR_REG  (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x48))
#define AUX_MU_LCR_REG  (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x4c))
#define AUX_MU_MCR_REG  (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x50))
#define AUX_MU_LSR_REG  (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x54))
#define AUX_MU_CNTL_REG (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x60))
#define AUX_MU_BAUD_REG (*(volatile uint32_t*)(CONFIG_AUX_BASE + 0x68))

#define LSR_RX_READY    0x01
#define LSR_TX_EMPTY    0x20

static int initialised = 0;

void auxuart_init(void)
{
	if (initialised)
		return;
	AUX_ENABLES    |= 1;
	AUX_MU_CNTL_REG = 0;
	AUX_MU_IER_REG  = 0;
	AUX_MU_LCR_REG  = 3;
	AUX_MU_MCR_REG  = 0;
	AUX_MU_IIR_REG  = 0xC6;
	AUX_MU_BAUD_REG = 270;
	AUX_MU_CNTL_REG = 3;
	initialised = 1;
}

void auxuart_putc(unsigned char c)
{
	if (!initialised)
		auxuart_init();
	while (!(AUX_MU_LSR_REG & LSR_TX_EMPTY)) { }
	AUX_MU_IO_REG = c;
}

int auxuart_getc_nb(void)
{
	if (!initialised)
		auxuart_init();
	if (AUX_MU_LSR_REG & LSR_RX_READY)
		return (int)(AUX_MU_IO_REG & 0xff);
	return -1;
}

unsigned char auxuart_getc(void)
{
	int b;
	while ((b = auxuart_getc_nb()) < 0) { }
	return (unsigned char)b;
}
