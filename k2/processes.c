#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#define DISPLAY 1
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




void test_nameserver(){
	char* myname = "test_nameserver";
	// set coulor to green
	uart_printf(CONSOLE, "\033[32m");
	uart_printf(CONSOLE, "test_nameserver: My name is %s\r\n", myname);
	// set coulor to white
	
	//call register
	uart_printf(CONSOLE, "test_nameserver: Registering my name\r\n");
	uart_printf(CONSOLE, "\033[37m");
	RegisterAs(myname);
	//call whois
	// set coulor to green
	uart_printf(CONSOLE, "\033[32m");
	uart_printf(CONSOLE, "test_nameserver: Calling whois on myself\r\n");
	// set coulor to white
	uart_printf(CONSOLE, "\033[37m");
	int tid = WhoIs(myname);
	// set coulor to green
	uart_printf(CONSOLE, "\033[32m");
	uart_printf(CONSOLE, "test_nameserver: tid = %u\r\n", tid);
	// set coulor to white
	uart_printf(CONSOLE, "\033[37m");
	Exit();

}
void player1(){
	int tid = MyTid();
	// register
	RegisterAs("player1");
	signup();
	for (int i = 0; i < 5; i++)
	{
		// do nothing
		char play_ret = play("rock");
		// play rock paper scissors
		uart_printf(CONSOLE, "Play: %u, player: %d have: ",i, tid);
		uart_putc(CONSOLE, (char)play_ret);
		uart_printf(CONSOLE, "\r\n");
	}

	quit();
	Exit();
}
void player2(){
	int tid = MyTid();
	// register
	RegisterAs("player2");
	signup();
	// play rock paper scissors
	for (int i = 0; i < 5; i++)
	{
		// do nothing
		char play_ret = play("paper");
		// play rock paper scissors
		uart_printf(CONSOLE, "Play %u, player:%d have: ",i, tid);
		uart_putc(CONSOLE, (char)play_ret);
		uart_printf(CONSOLE, "\r\n");
	}
	quit();
	Exit();
}
void sender(){
	int msglen = 10;
	int tid = MyTid();
    // Register
    RegisterAs("SENDER");
	// write a message
	uart_printf(CONSOLE, "k2_sender: My tid is %u\r\n", tid);
	char msg[msglen];
	for (int i = 0; i < msglen; i++)
	{
		msg[i] = 'B' + i;
	}
	msg[msglen - 1] = 0;
	// define the send message here
	tid = WhoIs("RECEIVER");
	char msgreply[msglen];
	
	// begin of the send
	int ret_code = Send(tid, msg, msglen, msgreply, msglen);
	uart_printf(CONSOLE, "k2_sender: Message sent, my reply is [%s] ret_code = %d \r\n", msgreply, ret_code);
	// end of the send
	Exit();
}
void receiver(){
	int msglen = 10;
	// initially this task should have tid of 3
	// recieve a message
	int mytid = MyTid();
    RegisterAs("RECEIVER");
	int tid;
	char msg[msglen];
	char reply[msglen];
	for (int i = 0; i < msglen; i++)
	{
		reply[i] = 'A' + i;
	}
	msg[msglen - 1] = 0;
	uart_printf(CONSOLE, "k2_receiver: My tid is %u\r\n", mytid);
	int recret = Receive(&tid, msg, msglen);
	uart_printf(CONSOLE, "k2_receiver: Message recieved: [%s], recret = %d\r\n", msg, recret);
	for (int i = 0; i < msglen; i++)
	{
		uart_putc(CONSOLE, reply[i]);
	}
	uart_printf(CONSOLE, "\r\n");

	// define the reply message here
	int repret = Reply(tid, reply, msglen);
	uart_printf(CONSOLE, "k2_receiver: Reply sent Repret = %d\r\n", repret);
	Exit();	
}
void first_task() // First task as dictated in the reqs
{
	// We are assuming that first_task has a priority of 2
	int tid = Create(1, sender);
	uart_printf(CONSOLE, "Created: %u\r\n", tid);
	tid = Create(1, receiver);
	uart_printf(CONSOLE, "Created: %u\r\n", tid);
	Exit();
}

