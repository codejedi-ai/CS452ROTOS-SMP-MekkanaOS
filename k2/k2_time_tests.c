#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
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

void CreateArgsTest(int i){
	RegisterAs("CreateArgsTest");
	uart_printf(CONSOLE, "CreateArgsTest: %u\r\n", i);
	Exit();
}

void parse_char_array(char *arr) {
  
  char *ptr; // pointer to traverse the array
  char *num[100]; // array to store the numbers
  int i = 1; // index for the array
  int used_length = 0;
  num[0] = arr;
  ptr = arr; // point to the first element of the array
  while (*ptr != '\0') { // loop until the end of the array
    if (*ptr == ' ') { // check if the character is a space
      *ptr = 0;
      num[i++] = ptr + 1; // store the value in the array
     // increment the index
    }
    ptr++; // move to the next character
    used_length++;
  }
  for (int j = 0; j < i; j++) {
	uart_printf(CONSOLE, "num[%d] = %s\r\n", j, num[j]);
  }
  
  //string compair
  char *command_cand = "CreateArgs";
  int cmp;
  strcmp(&cmp, num[0], "CreateArgs");
  if (cmp){
	  // this is a create args command
	  // the first argument is the priority
	  // the second argument is the function pointer
	  // the third argument is the number of arguments
	  // the fourth argument is the arguments
	  int argsno = 100;
	  int64_t args[100];
	  args[0] = 1768;
	  uart_printf(CONSOLE, "argsno: %x\r\n", argsno);
	  uart_printf(CONSOLE, "args: %x\r\n", args);
	  int tid = CreateArgs(5, CreateArgsTest, 100, args);
	  uart_printf(CONSOLE, "Created: %u\r\n", tid);
	  return;
  }
  
}
void recieve_task(){
	RegisterAs(".");
	// first it would await for a message to arrive that would ultimatelly be a command to initialize the task

}
void k2(){
	// register the k2
	RegisterAs("K2");
	unsigned int counter=1;
	char command[50];
	int command_length = 0;
	command[0] = '\0';
	uart_printf(CONSOLE, "PI[%u]> ", counter++);
	char c = ' ';
	while (c != 'q') {
		c = uart_getc(CONSOLE);
		if (c == '\r') {
			uart_printf(CONSOLE, "\r\n");
			parse_char_array(command);
			command_length = 0;
			command[0] = '\0';
			uart_printf(CONSOLE, "\r\nPI[%u]> ", counter++);
			Yield();
		}else if (c == '\b'){
			if (command_length > 0){
				command_length--;
				command[command_length] = '\0';
				uart_printf(CONSOLE, "\b \b");
			}
		}else {
			command[command_length] = c;
			command_length++;
			command[command_length] = '\0';
			uart_putc(CONSOLE, c);
		}
	}
	uart_puts(CONSOLE, "\r\n");
	Exit();
}