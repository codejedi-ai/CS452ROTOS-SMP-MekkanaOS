#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

/*
 * Build-mode configuration. The Makefile passes MARKLIN_HW_UART3 via -D:
 *   MODE=hw   -> MARKLIN_HW_UART3=1, BUILD_HW=1
 *   MODE=qemu -> MARKLIN_HW_UART3=0, BUILD_QEMU=1
 *
 * On real hardware the Marklin link is PL011 UART3 @ 0xFE201600, 2400 baud,
 * 8N2, hardware-flow-controlled. QEMU's raspi4b machine does NOT model UART3
 * as a second PL011, so in the QEMU build the link is routed through the
 * BCM2835 AUX mini-UART @ 0xFE215000, polled.
 */

#ifndef MARKLIN_HW_UART3
#define MARKLIN_HW_UART3 1  /* assume real hardware unless told otherwise */
#endif

#define CONFIG_MMIO_BASE   0xFE000000
#define CONFIG_UART0_BASE  (CONFIG_MMIO_BASE + 0x201000)
#define CONFIG_UART3_BASE  (CONFIG_MMIO_BASE + 0x201600)
#define CONFIG_AUX_BASE    (CONFIG_MMIO_BASE + 0x215000)
#define CONFIG_GPIO_BASE   (CONFIG_MMIO_BASE + 0x200000)

#endif /* _CONFIG_H_ */
