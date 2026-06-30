#include "io_notifier.h"
#include "io_api.h"
#include "UART1_CONSOLE_server.h"
#include "processes.h"
#include "rpi.h"
#include "syscall.h"
#include "nameserver.h"

void io_notifier(void)
{
	RegisterAs("io_notifier");

	int uart1_console_tid = -1;

	while (1) {
		uint64_t event = AwaitEvent(UARTINTER);
		int ret;

		if (uart1_console_tid <= 0)
			uart1_console_tid = WhoIs(UART1_CONSOLE_SERVER);

		uint8_t channel = (event >> 8) & 0xFF;

		if (channel == CONSOLE && uart1_console_tid > 0)
			Send(uart1_console_tid, (const char *)&event, 8, (char *)&ret, 0);
	}
	Exit();
}
