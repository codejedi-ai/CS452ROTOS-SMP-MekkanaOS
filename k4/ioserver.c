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
		uart_printf(CONSOLE, "io_notifier: 0x%x ", event);
		uart_printf(CONSOLE, "recieved message type = %d, channel = %d, char_ch = ", type, channel);
		uart_putc(CONSOLE, char_ch);
		uart_printf(CONSOLE, "\r\n");
		
		Send(WhoIs("io_server"), &event, 8, &ret, 0);
	}
	Exit();
}
void io_server() 
{
	uart_printf(CONSOLE, "io_server: Registered at %u\n", MyTid());
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
	uint8_t STATE[3]; // 0 is nothing, 1 is console, 2 is train (marklin)
	uint8_t send_queue[255]; 
	uint8_t send_queue_size = 0;
	uint8_t send_queue_begin = 0;
	uint8_t send_queue_end = 0;
	STATE[MARKLIN] = 0; // state 0 means READY, 1 means sent, 2 means marklin is busy
	int i = 0;
	uint8_t caller_TID = 0;
	while (1)
	{
		// recieve the message
		char recieve[8];
		Receive(&tid, recieve, 8);
		
		uint8_t type = recieve[0];
		uint8_t channel = recieve[1];
		char char_ch = recieve[2];
		uart_putc(CONSOLE, char_ch);
		uart_printf(CONSOLE, "\r\n");
		if (type != GETC && type != PUTC){
			// if (WhoIs("io_notifier") == tid) uart_printf(CONSOLE, "io_server: io_notifier called me!! Gotta reply to it\r\n");
			Reply(tid, recieve, 8);
		}
		if (type == CTSMIM){
			// CTS 0 means you cannot send
			uart_printf(CONSOLE, "io_server: CTS = %d\r\n", STATE[channel]);
			if(STATE[channel] == 1){
				// Set it to two the marklin is busy
				STATE[channel] = 2;
				uart_printf(CONSOLE, "STATE[%d] = %u\r\n",channel,  STATE[channel]);
			} else if(STATE[channel] == 2){
				STATE[channel] = 0;
				uart_printf(CONSOLE, "STATE[%d] = %u\r\n",channel,  STATE[channel]);
				Reply(caller_TID, recieve, 8);
			}
			
		}
		if(type == TXIC){
			// the transmit fires if the message finnished transmitting
			uart_printf(CONSOLE, "io_server: TXIC INTURRUPT\r\n");
			if(STATE[channel] == 0){
				// the char is sent
				STATE[channel] = 1;
				uart_printf(CONSOLE, "STATE[%d] = %u\r\n",channel,  STATE[channel]);

				// print channel STATE[channel]
				uart_printf(CONSOLE, "io_server: The message is sent and Marklin is ready to recieve\r\n");
				// relpy to the caller
			}
			// command send
			
		} else if(type == RXIC){
			// the recieve when the marklin have send a character
			uart_printf(CONSOLE, "io_server: RXIC INTURRUPT\r\n");
			//
		} else if(type == GETC){
			caller_TID = tid;
			// the server
		} else if(type == PUTC){
			uart_printf(CONSOLE, "STATE[%d] = %u\r\n",channel,  STATE[channel]);
			if(STATE[channel] == 0){
				caller_TID = tid;
				// the server
				uart_putc(channel, char_ch);
				uart_printf(CONSOLE, "io_server: PUT called, \'%c\' is to be printed.\r\n", char_ch);
			}
		}

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
	uart_printf(CONSOLE, "Putc: sendret = %d\r\n", sendret);
	return sendret;
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