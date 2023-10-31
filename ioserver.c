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
"\033[2J"	STATE the screen.
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

#define uartINTER 153
#define QUEUELENGTH 100
io_logging = 0;
// this struct can be used to store the function call and interrupts
struct intFun
{
	uint8_t tid;	  // the PID of the send, put or CTS
	uint8_t type;	  // the type of the interrupt it is waiting for
	uint8_t channel;  // the channel of the interrupt it is waiting for
	uint8_t char_ch;  // the character that is being sent or recieved or the state of the CTS it is waiting for
	uint8_t char_ch2; // the character that is being sent or recieved or the state of the CTS it is waiting for if it is a 2 character marklin command
};
// this list contains all the functions that are waiting for a certain interrupt, if that interrupt fires then the function is unblocked
// all functions that are blocked by the same interrupt are stored in a list
struct fi_list
{
	struct intFun call[QUEUELENGTH];
	int size;
	uint8_t begin;
	uint8_t end;
};
void set_io_logging(int val)
{
	io_logging = val;
}
// one queue to wait to be fired, another queue to await for the send to be returned
void io_TXIC_MARKLIN_server()
{
	// It is automatically assumed the channel is 2
	struct fi_list call_list;
	struct fi_list interrupt_list;
	RegisterAs("io_TXIC_MARKLIN_server");
	int8_t io_notifier_tid = WhoIs("io_notifier");
	// set the size of the lists to 0
	// define STATE car
	// STATE: 0 not STATE: 1 STATE
	// set call list size to 0
	// set interrupt list size to 0
	call_list.size = 0;
	interrupt_list.size = 0;
	uint8_t STATE = 1;
	while (1)
	{
		int tid;
		char recieve[8];
		Receive(&tid, recieve, 8);
		uint8_t type = recieve[0];
		uint8_t channel = recieve[1];
		uint8_t char_ch = recieve[2];
		uint8_t char_ch2 = recieve[3];
		
		if (io_notifier_tid == tid){
			Reply(tid, recieve, 0);
		}

		if(type == TXIC){
			// enqueue the interrupt
			// pop the call list queue and reply to the task
			// is there another bullet the soldier can fire
			// soldier <tid>'s bullet has landed
			
			// add interrupt to the interrupt list
			interrupt_list.call[interrupt_list.end].tid = tid;
			interrupt_list.call[interrupt_list.end].type = type;
			interrupt_list.call[interrupt_list.end].channel = channel;
			interrupt_list.call[interrupt_list.end].char_ch = char_ch;
			interrupt_list.call[interrupt_list.end].char_ch2 = char_ch2;
			interrupt_list.end = (interrupt_list.end + 1) % QUEUELENGTH;
			interrupt_list.size++;	
		} 
		
		if(type == PUTC){
			uart_printf(CONSOLE, "	PUTC FUNCTION tid = %u, char_ch = %u, char_ch2 = %u\r\n", channel, tid, char_ch, char_ch2);
			// enqueue the function call
			// print soldier <tid> entered firing queue print it a Kernel is like a military
			uart_printf(CONSOLE, "	soldier %u entered firing queue. %u already in.\r\n", tid, call_list.size);
			call_list.call[call_list.end].tid = tid;
			call_list.call[call_list.end].type = type;
			call_list.call[call_list.end].channel = channel;
			call_list.call[call_list.end].char_ch = char_ch;
			call_list.call[call_list.end].char_ch2 = char_ch2;
			call_list.end = (call_list.end + 1) % QUEUELENGTH;
			call_list.size++;
		}
		// if there exist an interrupt to match up with a request
		if (call_list.call[call_list.begin].tid != 0 && call_list.size > 0 && STATE == 1)
		{
			STATE = 0;
			uart_putc(MARKLIN, call_list.call[call_list.begin].char_ch);
			// print in green
			uart_printf(CONSOLE, "\033[32m");
			uart_printf(CONSOLE, "	soldier %u fired. %u left.\r\n", call_list.call[call_list.begin].tid, call_list.size - 1);
			// print in white
			uart_printf(CONSOLE, "\033[37m");
		}
		
		if(call_list.size > 0 && interrupt_list.size > 0){
			int ret_pid = call_list.call[call_list.begin].tid;
			recieve[0] = interrupt_list.call[interrupt_list.begin].type;
			recieve[1] = interrupt_list.call[interrupt_list.begin].channel;
			recieve[2] = interrupt_list.call[interrupt_list.begin].char_ch;
			recieve[3] = interrupt_list.call[interrupt_list.begin].char_ch2;
			uart_printf(CONSOLE, "\033[37m");
			uart_printf(CONSOLE, "	soldier %u returned. %u left.\r\n", ret_pid, call_list.size - 1);
			// print in green
			Reply(ret_pid, recieve, 8);
			call_list.call[call_list.begin].tid = 0;
			call_list.begin = (call_list.begin + 1) % QUEUELENGTH;

			call_list.size--;
			interrupt_list.begin = (interrupt_list.begin + 1) % QUEUELENGTH;
			interrupt_list.size--;
			STATE = 1;
		}
	}
	
	Exit();
}
void io_RXIC_MARKLIN_server()
{
	// It is automatically assumed the channel is 2
	int tid;
	RegisterAs("io_RXIC_MARKLIN_server");
	int io_notifier_tid = WhoIs("io_notifier");
	// for the recieve interrupts I need to handle cases in which the interrupt happened before a task picked it up
	// this doubles of as a queue for the interrupts
	struct fi_list interrupt_list;
	struct fi_list call_list;
	// set the size of the lists to 0
	// 0 is nothing
	// 1 is CONSOLE
	// 2 is MARKLIN

	// need to consider cases in which the interrupt arrived before the task picked it up
	// in other words the task is still blocked on the AwaitEvent or a PutC
	while (1)
	{
		// change font to orange
		// uart_printf(CONSOLE, "\033[33m");
		char recieve[8];
		Receive(&tid, recieve, 8);
		uint8_t type = recieve[0];
		uint8_t channel = recieve[1];
		uint8_t char_ch = recieve[2];
		// get arrives after interrupt
		// interrupt arrives after get
		if (io_notifier_tid == tid){
			Reply(tid, recieve, 0);
		}
		if (type == RXIC)
		{
			interrupt_list.call[interrupt_list.end].tid = tid;
			interrupt_list.call[interrupt_list.end].type = type;
			interrupt_list.call[interrupt_list.end].channel = channel;
			interrupt_list.call[interrupt_list.end].char_ch = char_ch;
			interrupt_list.end = (interrupt_list.end + 1) % QUEUELENGTH;
			interrupt_list.size++;
		}
		else if (type == GETC)
		{
			// check is the channel is empty
			// enqueue the interrupt
			// uart_printf(CONSOLE, "GETC FUNCTION char_ch = 0x%x, tid = %u\r\n", char_ch, tid);
			call_list.call[call_list.end].tid = tid;
			call_list.call[call_list.end].type = type;
			call_list.call[call_list.end].channel = channel;
			call_list.call[call_list.end].char_ch = char_ch;
			call_list.end = (call_list.end + 1) % QUEUELENGTH;
			call_list.size++;
		}
		// if there exist an interrupt to match up with a request
		if (call_list.size > 0 && interrupt_list.size > 0)
		{
			int ret_pid = call_list.call[call_list.begin].tid;
			recieve[0] = interrupt_list.call[interrupt_list.begin].type;
			recieve[1] = interrupt_list.call[interrupt_list.begin].channel;
			recieve[2] = interrupt_list.call[interrupt_list.begin].char_ch;
			recieve[3] = interrupt_list.call[interrupt_list.begin].char_ch2;
			// uart_printf(CONSOLE, "\033[37m");
			Reply(ret_pid, recieve, 8);
			call_list.call[call_list.begin].tid = 0;
			call_list.begin = (call_list.begin + 1) % QUEUELENGTH;
			call_list.size--;
			interrupt_list.begin = (interrupt_list.begin + 1) % QUEUELENGTH;
			interrupt_list.size--;
		}
		// print in white
		// uart_printf(CONSOLE, "\033[37m");
	}
	Exit();
}
void io_CTS_MARKLIN_server()
{
	// It is automatically assumed the channel is 2
	// awaitcts[0] is awaiting for down, awaitcts[1] is awaiting for up
	uint32_t awaitcts[2][NUMPROCS]; 
	
	// awaitcts_size[0] is the size of the list that is awaiting for down, 
	// awaitcts_size[1] is the size of the list that is awaiting for up
	uint32_t awaitcts_size[2]; 
	// register the server
	RegisterAs("io_CTS_MARKLIN_server");
	int io_notifier_tid = WhoIs("io_notifier");
	uint8_t STATE = 0;
	while (1)
	{
		int tid;
		char recieve[8];
		Receive(&tid, recieve, 8);
		uint8_t type = recieve[0];
		uint8_t channel = recieve[1];
		uint8_t char_ch = recieve[2];
		/* code */
		if (io_notifier_tid == tid){
			Reply(tid, recieve, 0);
		}
		if(type ==CTSMIM){
			uart_printf(CONSOLE, "	CTS = %d\r\n", char_ch);
			int cur_cts = get_CTS();
			for (int i = 0; i < NUMPROCS; i++){
				int tid_free = awaitcts[cur_cts][i];
				recieve[2] = cur_cts;
				Reply(tid_free, recieve, 8);
			}
			awaitcts_size[cur_cts] = 0;
		} else if(type == CTS){
			uart_printf(CONSOLE, "	CTS FUNCTION tid = %u, char_ch = %u\r\n", channel, tid, char_ch);
			if (char_ch == get_CTS()){
				Reply(tid, recieve, 8);
			} else {
				awaitcts[char_ch] = tid;
				awaitcts_size[char_ch]++;
			}
			// enqueue the function call
			// print soldier <tid> entered firing queue print it a Kernel is like a military
			// this is different in essence that it would 
		}
	}
	
	Exit();
}

void io_notifier()
{
	// The handler in the kernel only handels different type of interrupt of the same ID
	// However in between different interrupts of the same ID, it is handeled in here
	RegisterAs("io_notifier");
	int io_TXIC_MARKLIN_server_tid = Create(0, io_TXIC_MARKLIN_server);
	int io_RXIC_MARKLIN_server_tid = Create(0, io_RXIC_MARKLIN_server);
	int io_CTS_MARKLIN_server_tid = Create(0, io_CTS_MARKLIN_server);
	while (1)
	{
		uint64_t event = AwaitEvent(uartINTER);
		int ret;
		// the 0 th byte is the interrupt id

		uint8_t type = event & 0xFF;
		uint8_t channel = (event >> 8) & 0xFF;
		uint8_t char_ch = (event >> 16) & 0xFF;
		if (channel == CONSOLE){
			// there exist no server for the console
		}
		if (channel == MARKLIN)
		{
			if (type == RXIC)
			{
				uart_printf(CONSOLE, "RXIC SYSINTERRUPT\r\n");
				Send(io_RXIC_MARKLIN_server_tid, &event, 8, &ret, 0);
			}
			else if(type == TXIC)
			{
				uart_printf(CONSOLE, "TXIC SYSINTERRUPT\r\n");
				Send(io_TXIC_MARKLIN_server_tid, &event, 8, &ret, 0);
			}
			else if(type == CTSMIM)
			{
				uart_printf(CONSOLE, "CTSMIM SYSINTERRUPT\r\n");
				Send(io_CTS_MARKLIN_server_tid, &event, 8, &ret, 0);
			}
		}
	}
	Exit();
}

// was thinking about doing a three server layout. It is possible that two tasks are waiting for the same interrupt
void io_TXIC_MARKLIN_server_old()
{
/*
	if (io_logging)
		// uart_printf(CONSOLE, "io_TXIC_MARKLIN_server: Registered at %u\n", MyTid());
	RegisterAs("io_TXIC_MARKLIN_server");
	int io_notifier_tid = WhoIs("io_notifier");
  	int tid = 0;

	uint8_t await_cts_val[3];
	uint8_t STATE = 0;


	uint8_t tid_ret = 0;

	while (1)
	{
		// recieve the message
		char recieve[8];
		Receive(&tid, recieve, 8);
		
		uint8_t type = recieve[0];
		uint8_t channel = recieve[1];
		
		char char_ch = recieve[2];
		// yellow character
		// uart_printf(CONSOLE, "\033[33m");
		if (type != GETC && type != PUTC && type != CTS){
			if (WhoIs("io_notifier") == tid) // uart_printf(CONSOLE, "io_server: UART INTERRUPT io_notifier called me!! type = %u, channel = %u, char_ch = %u\r\n", type, channel, char_ch);
			
			Reply(tid, recieve, 8);
		}

		if (type == CTSMIM){

		} else if(type == TXIC){
			if(STATE == 1) {
				STATE = 0;
				recieve[2] = 1;
				Reply(tid_ret, recieve, 8);
			}
		} else if(type == PUTC){
			if(STATE == 0){
				uart_printf(CONSOLE, "PUTC FUNCTION channel = %u, tid = %u\r\n", channel, tid_ret);
				tid_ret = tid;
				STATE = 1;
				uart_putc(channel, char_ch);
				if (recieve[3] != -1){
					uart_putc(channel, recieve[3]);
				}
			} else {
				recieve[2] = -1;
				Reply(tid, recieve, 8);
			}
		}
	}
*/
	Exit();
}

/*
int Getc(int tid, int channel)
returns the next un-returned character from the given channel.
The first argument is the task id of the appropriate I/O server.
How communication errors are handled is implementation-dependent.
Getc() is actually a wrapper for a send to the appropriate server.
Return Value
>=0	new character from the given uart.
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
int Getc(int tid, int channel)
{
	char channel64[8];
	// uint8_t channel8 = (uint8_t) channel;
	// *((uint32_t *) channel64 + 1) = ((uint32_t) channel);
	channel64[0] = GETC;
	channel64[1] = (uint8_t)channel;
	channel64[2] = 0;
	channel64[3] = -1;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 8);
	if (io_logging)
		// uart_printf(CONSOLE, "GETC: sendret = %d\r\n", sendret);
	return channel64[2];
}
/*
int Putc(int tid, int channel, unsigned char ch)
queues the given character for transmission by the given uart.
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
int Putc(int tid, int channel, unsigned char ch)
{
	char channel64[8];
	*((uint32_t *)channel64 + 1) = ((uint32_t)channel);
	channel64[0] = PUTC;
	channel64[1] = (uint8_t)channel;
	channel64[2] = ch;
	channel64[3] = -1;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 8);
	//if (io_logging)
	uart_printf(CONSOLE, "Putc: sendret = %d\r\n", sendret);
	return channel64[2];
}
// cannot get over the waitCTS thing. I want the my code to unblock when the CTS is high
// this way the marklin would not swallow commands too fast
int Put2c(int tid, int channel, unsigned char ch, unsigned char ch2)
{
	char channel64[8];
	*((uint32_t *)channel64 + 1) = ((uint32_t)channel);
	channel64[0] = PUTC;
	channel64[1] = (uint8_t)channel;
	channel64[2] = ch;
	channel64[3] = ch2;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 1);
	//if (io_logging)
	uart_printf(CONSOLE, "Put2c: sendret = %d\r\n", sendret);
	return channel64[2];
}

int awaitCTS(int tid, int channel, uint8_t val)
{	
	// print the params
	uart_printf(CONSOLE, "awaitCTS: tid = %u, channel = %u, val = %u\r\n", tid, channel, val);
	char channel64[8];
	*((uint32_t *)channel64 + 1) = ((uint32_t)channel);
	channel64[0] = CTS;
	channel64[1] = (uint8_t)channel;
	channel64[2] = val;
	channel64[3] = -1;
	uint64_t sendret = Send(tid, &channel64, 8, &channel64, 8);
	uart_printf(CONSOLE, "awaitCTS: sendret = %d\r\n", sendret);
	return channel64[2];
}