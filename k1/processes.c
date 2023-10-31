#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#define DISPLAY 1
// Send message test. Process one would be sending a message to process two. 
// The expected output of this test would be seeing the message from process one in process two.
// The name of process one and process two would be abbreviared as k2p1 and k2p2
void k2p1(){
	int tid = MyTid();
	// write a message
	uart_printf(1, "k2p1: My tid is %u\r\n", tid);
	char *msg = "k2p1: Hello, this is process k2p1";
	int msglen = 24;
	tid = 3;
	char msgreply[50];
	int ret_code = Send(tid, msg, msglen, msgreply, 25);
	uart_printf(1, "k2p1: Message sent however my reply should be %s here..... ret_code = %u \r\n", msgreply, ret_code);
	Exit();
}
void k2p2(){
	// initially this task should have tid of 3
	// recieve a message
	int mytid = MyTid();
	int tid;
	char msg[50];
	int msglen = 48;
	uart_printf(1, "k2p2: My tid is %u\r\n", mytid);
	Receive(&tid, msg, msglen);
	uart_printf(1, "k2p2: Message recieved: %s\r\n", msg);
	char *reply = "k2p2: Hello, this is process k2p2 Hello There";
	Reply(tid, reply, 25);
	

	Exit();
}
void first_task() // First task as dictated in the reqs
{
	// We are assuming that first_task has a priority of 2
	int tid;
	tid = Create(1, k2p1);
	uart_printf(1,"Created: %u\r\n", tid);
	tid = Create(1, k2p2);
	uart_printf(1,"Created: %u\r\n", tid);
	Exit();
}