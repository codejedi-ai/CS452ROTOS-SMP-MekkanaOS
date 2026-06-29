#ifndef _DISPLAY_SERVER_H_
#define _DISPLAY_SERVER_H_ 1

/*
 * Display server: serializes all user-mode UART output through a single task.
 *
 * Why: scattered uart_printf calls from many user tasks interleave bytes on
 * the wire, and each call busy-spins on the TX FIFO inside whichever task
 * is running. Centralizing through this server means a) output is always
 * coherent, b) sender tasks don't burn cycles waiting for UART TX, c) we
 * have one place to add buffering / log levels later.
 *
 * Wire protocol over Send/Reply:
 *   client packs (DISPLAY_PRINT, payload bytes...) into the send buffer.
 *   server Replies with a single zero byte once it has written to the UART.
 *
 * Use:
 *   display_puts(str)          -> single string
 *   display_printf(fmt, ...)   -> printf-style, very small subset
 *   display_at(row, col, str)  -> cursor-positioned string
 *
 * Kernel-side prints (in syscall.c) bypass the server and call uart_printf
 * directly -- the kernel cannot Send.
 */

void display_server(void);    /* task entry */

void display_puts(const char *s);
void display_printf(const char *fmt, ...);
void display_at(int row, int col, const char *s);

#endif /* _DISPLAY_SERVER_H_ */
