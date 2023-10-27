#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "k2TimeTests.h"
#include "k2rps.h"
#include "k3tests.h"
#include "clockserver.h"

#include "asm.h"
#include "ioserver.h"
#define DISPLAY 1
#define GETC 32
#define PUTC 33
#define CTS 34

/*
These are the most essential terminal control sequences that you will need for your train program.

Code	Effect
"\033[2J"	Clear the screen.
"\033[H"	Move the cursor to the upper-left corner of the screen.
"\033[r;cH"	Move the cursor to row r, column c. Note that both the rows and columns are indexed starting at 1.
"\033[?25l"	Hide the cursor.
"\033[K"	Delete everything from the cursor to the end of the line.
These control sequences can help make your program's display more lively.

Code	Effect
"\033[0m"	Reset special formatting (such as colour).
"\033[30m"	Black text.
"\033[31m"	Red text.
"\033[32m"	Green text.
"\033[33m"	Yellow text.
"\033[34m"	Blue text.
"\033[35m"	Magenta text.
"\033[36m"	Cyan text.
"\033[37m"	White text.

*/


#define UARTINTER 153




void io_notifier(){
	RegisterAs("io_notifier");
	// uart_printf(CONSOLE, "io_notifier:Registered\n");
	while (1)
	{
		uint64_t event = AwaitEvent(UARTINTER);
		int ret;
		// the 0 th byte is the interrupt id

		uint8_t type = event & 0xFF;
		uint8_t channel = (event >> 8) & 0xFF;
		char char_ch = (event >> 16) & 0xFF;
		// uart_printf(CONSOLE, "io_notifier: type = %u, channel = %u, char_ch = %u\r\n", type, channel, char_ch);
		Send(WhoIs("io_server"), &event, 8, &ret, 0);
	}
	Exit();
}
void io_server() 
{
	// uart_printf(CONSOLE, "io_server: Registered at %u\n", MyTid());
	RegisterAs("io_server");
	// First task as dictated in the reqs
	// need to set the timer interrupt
	// uart_printf(CONSOLE, "Timer C3: %u\r\n", get_timerC3());
	// the IO server listens to the UART interrupt and the user commands
	// it's instruction queue would be it's message queue
	// We are assuming that FirstUserTask has a priority of 1
	// start gameserver
	// RegisterAs("FirstUserTask");
  	int tid = 0;

	uint8_t tid_list[3][3];
	uint8_t await_cts_val[3];
	for (int i = 0; i < 3; i++)
	{
		tid_list[0][i] = 0;
		tid_list[1][i] = 0;
		tid_list[2][i] = 0;
	}
	uint8_t send_queue_size = 0;
	uint8_t send_queue_begin = 0;
	uint8_t send_queue_end = 0;
	int i = 0;
	uint8_t caller_TID_PUTC = 0, caller_TID_GETC = 0;
	while (1)
	{
		// recieve the message
		char recieve[8];
		Receive(&tid, recieve, 8);
		
		uint8_t type = recieve[0];
		uint8_t channel = recieve[1];
		char char_ch = recieve[2];
		// yellow character
		uart_printf(CONSOLE, "\033[33m");
		if (type != GETC && type != PUTC && type != CTS){
			if (WhoIs("io_notifier") == tid) uart_printf(CONSOLE, "io_server: io_notifier called me!! type = %u, channel = %u, char_ch = %u\r\n", type, channel, char_ch);
			
			Reply(tid, recieve, 8);
		}

		if (type == CTSMIM){
			uart_printf(CONSOLE, "CTS channel = %u, tid = %u CTS = %d\r\n", channel, tid_list[CTS - GETC][channel], char_ch);
			if(tid_list[CTS - GETC][channel] != 0){
				uart_printf(CONSOLE, "REPLIED: CTS channel = %u, tid = %u\r\n", channel, tid_list[CTS - GETC][channel]);
				recieve[2] = char_ch;
				Reply(tid_list[CTS - GETC][channel], recieve, 8);
				tid_list[CTS - GETC][channel] = 0;
			} 
		} else if(type == TXIC){
			uart_printf(CONSOLE, "PUTC channel = %u, tid = %u\r\n", channel, tid_list[PUTC - GETC][channel]);
			if(tid_list[PUTC - GETC][channel] != 0) {
				// print reply to channel and putc
				uart_printf(CONSOLE, "REPLIED: PUTC channel = %u, tid = %u\r\n", channel, tid_list[PUTC - GETC][channel]);
				recieve[2] = char_ch;
				Reply(tid_list[PUTC - GETC][channel], recieve, 8);
				tid_list[PUTC - GETC][channel] = 0;
			}
		} else if(type == RXIC){			
			// uart print replied to chanenl and geth
			uart_printf(CONSOLE, "GETC channel = %u, tid = %u\r\n", channel, tid_list[GETC - GETC]);
			if(tid_list[GETC - GETC][channel] != 0) {
				uart_printf(CONSOLE, "REPLIED: GETC channel = %u, tid = %u\r\n", channel, tid_list[GETC - GETC]);
				Reply(tid_list[GETC - GETC][channel], recieve, 8);
				tid_list[GETC - GETC][channel] = 0;
			}
		} else if(type == GETC){
			tid_list[type - GETC][channel] = tid;
		} else if(type == PUTC){
			tid_list[type - GETC][channel] = tid;
			uart_putc(channel, char_ch);
		} else if(type == CTS){
			tid_list[type - GETC][channel] = tid;
		}
		// print in white
		uart_printf(CONSOLE, "\033[37m");
	}
	Exit();
}
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
// get a character from the terminal, the server would reply with the character

// the send character would be in 8 bytes
// secment 0 - 31 into 8 bytes in bits
// 0 - 7: TYPE CTSMIM, RXIC, TXIC, GETC, PUTC...
// 8 - 15: Console
// 16 - 23: the return or put character
// 24 - 31
// 32 - 39
// 40 - 47
// 48 - 55
// 56 - 63 : Identifier bits, those bits identifies the command issued to the server
int Getc(int tid, int channel){
	char channel64[8];
	// uint8_t channel8 = (uint8_t) channel;
  	// *((uint32_t *) channel64 + 1) = ((uint32_t) channel);
	channel64[0] = GETC;
	channel64[1] = (uint8_t) channel;
	channel64[2] = 0;
	channel64[3] = -1;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 8);
	uart_printf(CONSOLE, "GETC: sendret = %d\r\n", sendret);
	return channel64[2];
}
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

// 0 - 7: TYPE CTSMIM, RXIC, TXIC, GETC, PUTC...
// 8 - 15: Console
// 16 - 23: the return or put character
// 24 - 31
// 32 - 39
// 40 - 47
// 48 - 55
// 56 - 63 : Identifier bits, those bits identifies the command issued to the server
int Putc(int tid, int channel, unsigned char ch){
	char channel64[8];
  	*((uint32_t *) channel64 + 1) = ((uint32_t) channel);
	channel64[0] = PUTC;
	channel64[1] = (uint8_t) channel;
	channel64[2] = ch;
	channel64[3] = -1;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 8);
	uart_printf(CONSOLE, "Putc: sendret = %d\r\n", sendret);
	return sendret;
}


int awaitCTS(int tid, int channel){
	char channel64[8];
  	*((uint32_t *) channel64 + 1) = ((uint32_t) channel);
	channel64[0] = CTS;
	channel64[1] = (uint8_t) channel;
	channel64[2] = -1;
	channel64[3] = -1;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 8);
	uart_printf(CONSOLE, "awaitCTS: sendret = %d\r\n", sendret);
	return channel64[2];
}