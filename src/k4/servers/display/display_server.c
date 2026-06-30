#include "display_server.h"
#include "UART1_CONSOLE_server.h"
#include "rpi.h"
#include "util.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include <stdarg.h>

// DISPLAY_PAYLOAD_MAX comes from display_server.h ((2 << 20) = 2 MiB).
// DISPLAY_PAYLOAD_OFF stays at 1 (one byte type tag at msg[0]).
#define DISPLAY_PAYLOAD_OFF 1
#define DISPLAY_ROWS 30
#define DISPLAY_COLS 120

static int display_tid = -1;
static char display_buffer[DISPLAY_ROWS][DISPLAY_COLS];
static int cursor_row = 0;
static int cursor_col = 0;

static int resolve_display_tid(void)
{
	if (display_tid < 0)
	{
		display_tid = WhoIs("display_server");
		while (display_tid < 0)
		{
			Yield();
			display_tid = WhoIs("display_server");
		}
	}
	return display_tid;
}

static void format_to_buffer(char *out, size_t cap, char *fmt, va_list va)
{
	char bf[12];
	char ch;
	size_t pos = 0;

	if (cap == 0)
		return;

	while ((ch = *(fmt++)) && pos + 1 < cap)
	{
		if (ch != '%')
		{
			out[pos++] = ch;
			continue;
		}
		ch = *(fmt++);
		switch (ch)
		{
		case 'u':
			ui2a(va_arg(va, unsigned int), 10, bf);
			for (char *p = bf; *p && pos + 1 < cap; p++)
				out[pos++] = *p;
			break;
		case 'd':
			i2a(va_arg(va, int), bf);
			for (char *p = bf; *p && pos + 1 < cap; p++)
				out[pos++] = *p;
			break;
		case 'x':
			ui2a(va_arg(va, unsigned int), 16, bf);
			for (char *p = bf; *p && pos + 1 < cap; p++)
				out[pos++] = *p;
			break;
		case 's':
		{
			char *s = va_arg(va, char *);
			while (*s && pos + 1 < cap)
				out[pos++] = *(s++);
			break;
		}
		case 'b':
			ui2a(va_arg(va, unsigned int), 2, bf);
			for (char *p = bf; *p && pos + 1 < cap; p++)
				out[pos++] = *p;
			break;
		case 'c':
			out[pos++] = (char)va_arg(va, int);
			break;
		case '%':
			out[pos++] = ch;
			break;
		case '\0':
			out[pos] = '\0';
			return;
		default:
			out[pos++] = ch;
			break;
		}
	}
	out[pos] = '\0';
}

static void server_print_sensors(int arr[], size_t alen, unsigned int column, unsigned int row)
{
	uart_printf(CONSOLE, "\033[33m");
	for (uint32_t i = 0; i < alen; i++)
	{
		if (arr[i] != 0)
		{
			uart_printf(CONSOLE, "\033[%u;%uH", row + i, column);
			char ch = 'A';
			uart_putc(CONSOLE, ch + (arr[i] / 17));
			uart_printf(CONSOLE, "%d", arr[i] % 17);
			uart_putc(CONSOLE, ' ');
		}
	}
	uart_printf(CONSOLE, "\033[0m");
}

static void server_puts(const char *s)
{
	while (*s) {
		char ch = *(s++);
		uart_putc(CONSOLE, (unsigned char)ch);

		if (ch == '\n') {
			cursor_row++;
			cursor_col = 0;
			if (cursor_row >= DISPLAY_ROWS)
				cursor_row = DISPLAY_ROWS - 1;
		} else if (ch == '\r') {
			cursor_col = 0;
		} else if (ch == '\b') {
			if (cursor_col > 0)
				cursor_col--;
		} else {
			if (cursor_col < DISPLAY_COLS) {
				display_buffer[cursor_row][cursor_col] = ch;
				cursor_col++;
			}
		}
	}
}

int DisplayServerTid(void)
{
	return resolve_display_tid();
}

// DISPLAY_PAYLOAD_MAX is 2 MiB (from display_server.h). A stack array that
// big would blow the per-task stack, so the puts/sends path uses a single
// static buffer. The other DisplayPrintf / DisplayPrintSensors callers
// have their own static buffers (defined below). All four are serialized
// by the fact that there's exactly one display_server task — Send blocks
// the caller until Reply, so the buffer ownership transfers cleanly.
static char display_puts_buf[DISPLAY_MSG_LEN];

int DisplayPuts(const char *s)
{
	int tid = resolve_display_tid();
	char reply;
	int rc = 0;

	display_puts_buf[0] = DISPLAY_PUTS;
	while (*s) {
		unsigned int n = 0;
		// Leave room for the NUL terminator.
		while (s[n] && n < (unsigned int)(DISPLAY_PAYLOAD_MAX - 1))
			n++;
		for (unsigned int i = 0; i < n; i++)
			display_puts_buf[DISPLAY_PAYLOAD_OFF + i] = s[i];
		display_puts_buf[DISPLAY_PAYLOAD_OFF + n] = '\0';
		rc = Send(tid, display_puts_buf, (int)(n + DISPLAY_PAYLOAD_OFF + 1), &reply, 0);
		if (rc < 0)
			return rc;
		s += n;
	}
	return rc;
}

// With DISPLAY_MSG_LEN now 2 MiB, every `char msg[DISPLAY_MSG_LEN]` on the
// stack would blow the 64 KiB per-task stack. Use one static buffer for
// each caller path. They share the same single display_server so the
// kernel's Send/Receive plumbing already serializes their use — a sender
// can only be re-entered once display_server replies, and the receiver
// loop reuses its own buffer.
static char display_printf_buf[DISPLAY_MSG_LEN];
static char display_sensors_buf[DISPLAY_MSG_LEN];
static char display_server_buf[DISPLAY_MSG_LEN];

void DisplayPrintf(char *fmt, ...)
{
	va_list va;
	char reply;

	va_start(va, fmt);
	display_printf_buf[0] = DISPLAY_PUTS;
	format_to_buffer(display_printf_buf + DISPLAY_PAYLOAD_OFF, DISPLAY_PAYLOAD_MAX, fmt, va);
	va_end(va);
	Send(resolve_display_tid(), display_printf_buf, DISPLAY_MSG_LEN, &reply, 0);
}

void DisplayPrintSensors(int arr[], size_t alen, unsigned int column, unsigned int row)
{
	char reply;
	size_t count = alen > 10 ? 10 : alen;
	size_t i;

	display_sensors_buf[0] = DISPLAY_PRINT_SENSORS;
	display_sensors_buf[1] = (uint8_t)row;
	display_sensors_buf[2] = (uint8_t)column;
	display_sensors_buf[3] = (uint8_t)count;
	for (i = 0; i < count; i++)
	{
		int v = arr[i];
		display_sensors_buf[4 + i * 4] = (char)(v & 0xff);
		display_sensors_buf[5 + i * 4] = (char)((v >> 8) & 0xff);
		display_sensors_buf[6 + i * 4] = (char)((v >> 16) & 0xff);
		display_sensors_buf[7 + i * 4] = (char)((v >> 24) & 0xff);
	}
	Send(resolve_display_tid(), display_sensors_buf, DISPLAY_MSG_LEN, &reply, 0);
}

void display_server(void)
{
	char *msg = display_server_buf;
	char reply;
	int sender;

	RegisterAs("display_server");

	while (1)
	{
		Receive(&sender, msg, DISPLAY_MSG_LEN);
		switch (msg[0])
		{
		case DISPLAY_PUTS:
			server_puts(msg + DISPLAY_PAYLOAD_OFF);
			break;
		case DISPLAY_PRINT_SENSORS:
		{
			int sensors[10];
			uint8_t count = msg[3];
			uint8_t row = msg[1];
			uint8_t col = msg[2];
			uint8_t i;

			if (count > 10)
				count = 10;
			for (i = 0; i < count; i++)
			{
				sensors[i] = (int)(msg[4 + i * 4] | (msg[5 + i * 4] << 8) |
						   (msg[6 + i * 4] << 16) | (msg[7 + i * 4] << 24));
			}
			server_print_sensors(sensors, count, col, row);
			break;
		}
		default:
			break;
		}
		Reply(sender, &reply, 0);
	}
}
