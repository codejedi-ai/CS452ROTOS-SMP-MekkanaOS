#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
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



// Send message test. Process one would be sending a message to process two. 
// The expected output of this test would be seeing the message from process one in process two.
// The name of process one and process two would be abbreviared as k2p1 and k2p2
void k2p1(){
	int tid = MyTid();
	// write a message
	uart_printf(CONSOLE, "k2p1: My tid is %u\r\n", tid);
	char *msg = "k2p1: Hello, this is process k2p1";
	int msglen = 24;
	tid = 4;
	char msgreply[50];
	int ret_code = Send(tid, msg, msglen, msgreply, 25);
	uart_printf(CONSOLE, "k2p1: Message sent however my reply should be [%s]..... ret_code = %d \r\n", msgreply, ret_code);
	Exit();
}
void k2p2(){
	// initially this task should have tid of 3
	// recieve a message
	int mytid = MyTid();
	int tid;
	char msg[50];
	int msglen = 48;
	uart_printf(CONSOLE, "k2p2: My tid is %u\r\n", mytid);
	int recret = Receive(&tid, msg, msglen);
	uart_printf(CONSOLE, "k2p2: Message recieved: [%s], recret = %d\r\n", msg, recret);
	char *reply = "k2p1 Hello There";
	int repret = Reply(tid, reply, 25);
	uart_printf(CONSOLE, "k2p2: Reply sent Repret = %d\r\n", repret);
	

	Exit();
}
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
	signup();
	char play_ret = play("rock");
	// play rock paper scissors
	uart_printf(CONSOLE, "player:%d have: ", tid);
	uart_putc(CONSOLE, (char)play_ret);
	uart_printf(CONSOLE, "\r\n");
	quit();
	Exit();
}
void player2(){
	int tid = MyTid();
	signup();
	// play rock paper scissors
	char play_ret = play("rock");
	uart_printf(CONSOLE, "player:%d have: ", tid);
	uart_putc(CONSOLE, (char)play_ret);
	uart_printf(CONSOLE, "\r\n");
	quit();
	Exit();
}
void first_task() // First task as dictated in the reqs
{
	// We are assuming that first_task has a priority of 2
	int tid;
	/*
	tid = Create(1, k2p1);
	uart_printf(CONSOLE,"Created: %u\r\n", tid);
	tid = Create(2, k2p2);
	uart_printf(CONSOLE,"Created: %u\r\n", tid);
	*/
	tid = Create(1, player1);
	uart_printf(CONSOLE,"Created: %u\r\n", tid);
	tid = Create(1, player2);
	uart_printf(CONSOLE,"Created: %u\r\n", tid);
	Exit();
}