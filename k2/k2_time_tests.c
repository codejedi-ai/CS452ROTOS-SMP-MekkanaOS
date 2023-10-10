#include "rpi.h"
#include "util.h"
#include "nameserver.h"
/*
For this test I would be conducting
opt or noopt - indicating whether optimization enabled or not
R or S - indicating receiver first or sender first
4, 64, or 256 - indicating message size
the measured time per SRR operation in microseconds
*/


// generate the task for recieve first then send

// Send message test. Process one would be sending a message to process two. 
// The expected output of this test would be seeing the message from process one in process two.
// The name of process one and process two would be abbreviared as k2_sender and k2_receiver
void k2_sender(int msglen){
	int tid = MyTid();
    // Register
    RegisterAs("SENDER");
	// write a message
	uart_printf(CONSOLE, "k2_sender: My tid is %u\r\n", tid);
	char msg[msglen];
	for (int i = 0; i < msglen; i++)
	{
		msg[i] = 'S';
	}
	// define the send message here
	tid = WhoIs("RECEIVER");
	char msgreply[msglen];
	int ret_code = Send(tid, msg, msglen, msgreply, msglen);
	uart_printf(CONSOLE, "k2_sender: Message sent however my reply should be [%s]..... ret_code = %d \r\n", msgreply, ret_code);
}
void k2_receiver(int msglen){
	// initially this task should have tid of 3
	// recieve a message
	int mytid = MyTid();
    RegisterAs("RECEIVER");
	int tid;
	char msg[msglen];
	for (int i = 0; i < msglen; i++)
	{
		msg[i] = 'R';
	}
	uart_printf(CONSOLE, "k2_receiver: My tid is %u\r\n", mytid);
	int recret = Receive(&tid, msg, msglen);
	uart_printf(CONSOLE, "k2_receiver: Message recieved: [%s], recret = %d\r\n", msg, recret);
	char reply[msglen];
	// define the reply message here
	int repret = Reply(tid, reply, msglen);
	uart_printf(CONSOLE, "k2_receiver: Reply sent Repret = %d\r\n", repret);	
}
// make sender and receiever of 4 message size
void k2_sender_4(){
	k2_sender(4);
	Exit();
}
void k2_receiver_4(){
	k2_receiver(4);
	Exit();
}
void recieve_first_send_second(){
    int tid = Create(1, k2_sender_4);
    uart_printf(CONSOLE,"Created: %u\r\n", tid);
    tid = Create(1, k2_receiver_4);
    uart_printf(CONSOLE,"Created: %u\r\n", tid);
    Exit();
}
void send_first_recieve_second(){
    int tid = Create(1, k2_receiver_4);
    uart_printf(CONSOLE,"Created: %u\r\n", tid);
    tid = Create(1, k2_sender_4);
    uart_printf(CONSOLE,"Created: %u\r\n", tid);
    Exit();
}
void k2(){
    // recieve_first_send_second();
    send_first_recieve_second();
    Exit();
}