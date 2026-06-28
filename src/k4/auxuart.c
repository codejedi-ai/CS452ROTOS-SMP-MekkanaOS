#include "auxuart.h"
#include "config.h"

/*
 * BCM2835 AUX mini-UART, polled. Used only by the QEMU build
 * (MARKLIN_HW_UART3 == 0). On real Pi 4 hardware the Marklin link is on
 * PL011 UART3 (see rpi.c) and this file does nothing at runtime.
 */

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

void aux_uart_init(void)
{
    if (initialised) return;
    AUX_ENABLES    |= 1;          /* enable mini-UART       */
    AUX_MU_CNTL_REG = 0;          /* disable RX/TX while we config */
    AUX_MU_IER_REG  = 0;          /* no interrupts          */
    AUX_MU_LCR_REG  = 3;          /* 8-bit mode             */
    AUX_MU_MCR_REG  = 0;
    AUX_MU_IIR_REG  = 0xC6;       /* clear FIFOs            */
    AUX_MU_BAUD_REG = 270;        /* ~115200 @ 500 MHz core, irrelevant in QEMU */
    AUX_MU_CNTL_REG = 3;          /* enable RX + TX         */
    initialised = 1;
}

void aux_uart_putc(unsigned char c)
{
    if (!initialised) aux_uart_init();
    while (!(AUX_MU_LSR_REG & LSR_TX_EMPTY)) { }
    AUX_MU_IO_REG = c;
}

unsigned char aux_uart_getc(void)
{
    if (!initialised) aux_uart_init();
    while (!(AUX_MU_LSR_REG & LSR_RX_READY)) { }
    return (unsigned char)(AUX_MU_IO_REG & 0xff);
}

int aux_uart_rxc(void)
{
    if (!initialised) aux_uart_init();
    return (AUX_MU_LSR_REG & LSR_RX_READY) ? 1 : 0;
}
