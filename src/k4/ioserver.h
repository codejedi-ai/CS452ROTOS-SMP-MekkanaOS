#include "rpi.h"
void io_TXIC_MARKLIN_server();
void io_RXIC_MARKLIN_server();
void io_CTS_MARKLIN_server();
void io_notifier();

/* Console (UART0, line 1) input. The PL011 RX interrupt does not route
   through GIC-400 reliably under QEMU raspi4b -- same root cause as the
   K3 system-timer IRQ. So console_rx_notifier polls uart_getc_queue and
   forwards bytes to io_RXIC_CONSOLE_server, which keeps a paired queue
   of pending bytes and pending Getc waiters. Shell calls
   Getc(server_tid, CONSOLE) and blocks until a key arrives. */
void io_RXIC_CONSOLE_server(void);
void console_rx_notifier(void);

/*
int Getc(int tid, int channel)
returns the next un-returned character from the given channel. 
The first argument is the task id of the appropriate I/O server. 
How communication errors are handled is implementation-dependent. 
Getc() is actually a wrapper for a send to the appropriate server.
Return Value
>=0	new character from the given UART.
-1	tid is not a valid uart server task.
*/
// have the server look out for the most recent interrupt that is the RXIC on the marklin
int Getc(int tid, int channel);
/*
int Putc(int tid, int channel, unsigned char ch)
queues the given character for transmission by the given UART. 
On return the only guarantee is that the character has been queued. 
Whether it has been transmitted or received is not guaranteed. 
How communication errors are handled is implementation-dependent. 
Putc() is actually a wrapper for a send to the appropriate server.
Return Value
0	success.
-1	tid is not a valid uart server task.
*/
// Either the queue is empty or the server needs to wait for the TXIC interrupt to be triggered
int Putc(int tid, int channel, unsigned char ch);
// int Put2c (int tid, int channel, unsigned char ch1, unsigned char ch2);
// DO NOT USE await CTS
int awaitCTS(int tid, int channel, uint8_t val);