#include "display_server.h"
#include "../rpi.h"
#include "syscall.h"
#include "nameserver.h"
#include "../util.h"
#include <stdarg.h>

#define DISPLAY_BUF_MAX 256

/* All wire messages are: [len_lo][len_hi][bytes...]. The server writes the
   bytes verbatim to UART0 and replies with a single 0 byte. */
struct disp_msg {
    uint16_t len;
    char     bytes[DISPLAY_BUF_MAX];
};

/* ------------------------- server task ------------------------------------ */
void display_server(void)
{
    RegisterAs("display");

    while (1) {
        int tid;
        struct disp_msg m;
        Receive(&tid, (char *)&m, sizeof(m));
        if (m.len > DISPLAY_BUF_MAX) m.len = DISPLAY_BUF_MAX;
        uart_putl(CONSOLE, m.bytes, m.len);
        char ack = 0;
        Reply(tid, &ack, 1);
    }
}

/* ------------------------- client helpers --------------------------------- */
static int g_display_tid = -1;
static int display_tid(void)
{
    if (g_display_tid < 0) g_display_tid = WhoIs("display");
    return g_display_tid;
}

static void send_buf(const char *buf, int len)
{
    if (len <= 0) return;
    if (len > DISPLAY_BUF_MAX) len = DISPLAY_BUF_MAX;
    struct disp_msg m;
    m.len = (uint16_t)len;
    for (int i = 0; i < len; i++) m.bytes[i] = buf[i];
    char ack;
    Send(display_tid(), (char *)&m, sizeof(uint16_t) + len, &ack, 1);
}

void display_puts(const char *s)
{
    int n = 0;
    while (s[n]) n++;
    send_buf(s, n);
}

/* Minimal printf: %d %u %x %s %c %%. Mirrors rpi.c's uart_format_print. */
static int strput(char *dst, int dst_max, int *dst_len, const char *src)
{
    while (*src) {
        if (*dst_len >= dst_max) return -1;
        dst[(*dst_len)++] = *src++;
    }
    return 0;
}

void display_printf(const char *fmt, ...)
{
    char buf[DISPLAY_BUF_MAX];
    int  len = 0;
    char numbuf[24];

    va_list va; va_start(va, fmt);
    char ch;
    while ((ch = *(fmt++)) && len < DISPLAY_BUF_MAX) {
        if (ch != '%') {
            buf[len++] = ch;
            continue;
        }
        ch = *(fmt++);
        switch (ch) {
            case 'u': ui2a(va_arg(va, unsigned int), 10, numbuf); strput(buf, DISPLAY_BUF_MAX, &len, numbuf); break;
            case 'd': i2a (va_arg(va, int),               numbuf); strput(buf, DISPLAY_BUF_MAX, &len, numbuf); break;
            case 'x': ui2a(va_arg(va, unsigned int), 16, numbuf); strput(buf, DISPLAY_BUF_MAX, &len, numbuf); break;
            case 'b': ui2a(va_arg(va, unsigned int),  2, numbuf); strput(buf, DISPLAY_BUF_MAX, &len, numbuf); break;
            case 's': strput(buf, DISPLAY_BUF_MAX, &len, va_arg(va, char *)); break;
            case 'c': if (len < DISPLAY_BUF_MAX) buf[len++] = (char)va_arg(va, int); break;
            case '%': if (len < DISPLAY_BUF_MAX) buf[len++] = '%'; break;
            case '\0': goto out;
            default:  /* unknown specifier: emit literally */
                if (len < DISPLAY_BUF_MAX) buf[len++] = '%';
                if (len < DISPLAY_BUF_MAX) buf[len++] = ch;
                break;
        }
    }
out:
    va_end(va);
    send_buf(buf, len);
}

void display_at(int row, int col, const char *s)
{
    display_printf("\033[%d;%dH%s", row, col, s);
}
