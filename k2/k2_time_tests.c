#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "processes.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "systimer.h"
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
/*

5.2.1. Measure multiple times
for (i=0;i<N;i++) {
  clock()
  something()
  clock()
}
gives you N measurements instead of 1
can use this to quantify/understand variability, or smooth it
e.g, compute variance, look for outliers
doesn’t help with overhead problem
doesn’t help with precision problem
void Measure_multiple_times(int N, void (*functionPtr) ()){
    int i;
    for (i = 0; i < N; i++){
        unsigned int start = get_timerLO();
        functionPtr();
        unsigned int end = get_timerLO();
        unsigned int time = end - start;
        uart_printf(CONSOLE, "Time taken for iteration %d is %u\r\n", i, time);
    }
    Exit();
}
/*
5.2.2. Measure something larger
start = clock()
for (i=0;i<N;i++) {
  something()
}
end = clock()
can smooth variability (via averaging) but does not help you measure/understand it
helps with overhead - increase N until overhead is insignificant
helps with precision - increase N until what you’re measuring is much slower than your clock

void Measure_something_larger(int N, void (*functionPtr) ()){
    unsigned int start = get_timerLO();
    int i;
    for (i = 0; i < N; i++){
        functionPtr();
    }
    unsigned int end = get_timerLO();
    unsigned int time = end - start;
    uart_printf(CONSOLE, "Time taken for iteration %d is %u\r\n", i, time);
    Exit();
}
*/


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
void k2_receiver(){
	// initially this task should have tid of 3
	// recieve a message
	
	// initially this task should have tid of 3
	// recieve a message
	int mytid = MyTid();
    RegisterAs("RECEIVER");
	// initialize task get the N and the msglen
	
	char init_msg[9];
	init_msg[8] = 0;
	// the init msg would take in two strings
	// the first one being N and the second one being msglen

	// READ THE N
	int init_rec = Receive(&mytid, init_msg, 8);
	// int parse_char_arr(char *arr, char **num, int num_size);
	int64_t N = atoi(init_msg);
	Reply(mytid, "INIT", 4);

	// READ THE MSGLEN
	init_rec = Receive(&mytid, init_msg, 8);
	// int parse_char_arr(char *arr, char **num, int num_size);
	int64_t msglen = atoi(init_msg);
	Reply(mytid, "INIT", 4);


	mytid = MyTid();
	// uart_printf(CONSOLE, "k2_receiver: My tid is %u, N = %u, msglen = %u\r\n", mytid, N, msglen);

	int tid;
	char msg[msglen];
	char reply[msglen];
	for (int i = 0; i < msglen; i++)
	{
		reply[i] = 'A' + (i + 1) % 26;
	}
	reply[msglen - 1] = 0;

	// uart_printf(CONSOLE, "k2_receiver: My tid is %u\r\n", mytid);
	for (int i = 0; i < N; i++){
		// define the reply message here
		//uart_printf(CONSOLE, "k2_receiver: Waiting for message %u\r\n", i);
		// begin of the send
		// uart_printf(CONSOLE, "k2_receiver: Waiting for message %u\r\n", i);
		int recret = Receive(&tid, msg, msglen);
		// uart_printf(CONSOLE, "k2_receiver: Message recieved: [%s], recret = %d\r\n", msg, recret);
		int repret = Reply(tid, reply, msglen);
		// uart_printf(CONSOLE, "k2_receiver: Reply sent Repret = %d\r\n", repret);
	}
	Exit();	
}

void CreateArgsTest(int i, int j){
	RegisterAs("CreateArgsTest");
	uart_printf(CONSOLE, "CreateArgsTest: %u, %u\r\n", i, j);
	Exit();
}
int64_t adder(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e, int64_t f, int64_t g, int64_t h, int64_t i, int64_t j) {
    return a + b + c + d + e + f + g + h + i + j;
}
// make a addrTask as a test for the adder function
void adderTask(int64_t a, int64_t b, int64_t c, int64_t d, int64_t e, int64_t f, int64_t g, int64_t h, int64_t i, int64_t j){
	// print the params
	RegisterAs("adderTask");
	uart_printf(CONSOLE, "adderTask: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\r\n", a, b, c, d, e, f, g, h, i, j);
	uart_printf(CONSOLE, "adderTask result: %u\r\n", adder(a, b, c, d, e, f, g, h, i, j));
	Exit();
}


void turnaround_test_parse_command(char *arr) {
  
  char *ptr; // pointer to traverse the array
  char *num[100]; // array to store the numbers
  int iter_no = 1; // index for the array
  int used_length = 0;
  num[0] = arr;
  ptr = arr; // point to the first element of the array
  while (*ptr != '\0') { // loop until the end of the array
    if (*ptr == ' ') { // check if the character is a space
      *ptr = 0;
      num[iter_no++] = ptr + 1; // store the value in the array
     // increment the index
    }
    ptr++; // move to the next character
    used_length++;
  }
  for (int j = 0; j < iter_no; j++) {
	uart_printf(CONSOLE, "num[%d] = %s\r\n", j, num[j]);
  }
  int cmp;
  strcmp(&cmp, num[0], "yield");
  if (cmp){
	  Yield();
	  return;
  }
  Yield();
  //string compair
  strcmp(&cmp, num[0], "testsendRF");
  if (cmp){
	// this is a send command
	// the first argument is the tid
	// the second argument is the message
	// create tasks
	if (iter_no < 3){
		uart_printf(CONSOLE, "Not enough arguments\r\n");
		return;
	}
	char args[1];

	int64_t N = atoi(num[1]);
	int64_t str_length = atoi(num[2]);

	// Init the reciever
	int tid = Create(2, k2_receiver);
	uart_printf(CONSOLE, "Created k2_receiver: %u\r\n", tid);
	Send(tid, num[1], str_length, args, 1); // send the N
	Send(tid, num[2], str_length, args, 1); // send the msglen
	// int tid = CreateArgs(5, k2_receiver, 1, args);
	//k2_sender(N, str_length);
	// send a message to the reciever
	// int tid = Create(5, k2_sender);


	char msg[str_length];
	for (int i = 0; i < str_length; i++)
	{
		msg[i] = 'A' + i % 26;
	}
	msg[str_length - 1] = 0;
	char msgreply[str_length];
	unsigned int start = get_timerLO();
	for (int i = 0; i < N; i++){
		
		//uart_printf(CONSOLE, "k2_sender: Sending message %u\r\n", i);
		Send(tid, msg, str_length, msgreply, str_length);
		//uart_printf(CONSOLE, "K2: Message sent, my reply is [%s] \r\n", msgreply);

	}
	
	unsigned int end = get_timerLO();
	unsigned int avgtime = (end - start) / N;
	// recreate the reciever task
	
	tid = Create(2, k2_receiver);
	// uart_printf(CONSOLE, "Created k2_receiver: %u\r\n", tid);
	Send(tid, num[1], str_length, args, 1); // send the N
	Send(tid, num[2], str_length, args, 1); // send the msglen
	// calculate the variance
	uint64_t variance = 0;
	// make the readings array 
	unsigned int readings[N];
	for (int i = 0; i < N; i++){
		unsigned int start = get_timerLO();
		Send(tid, msg, str_length, msgreply, str_length);
		unsigned int end = get_timerLO();
		unsigned int time = end - start;
		readings[i] = time;
	}
	
	// calculate the variance
	for (int i = 0; i < N; i++){
		variance += (readings[i] - avgtime) * (readings[i] - avgtime);
	}
	variance = variance / N;
	Yield();
	uart_printf(CONSOLE, "Average Time taken for %d characters is %u ", str_length, avgtime);
	Yield();
	uart_printf(CONSOLE, "Variance is %u\r\n", variance);
	Yield();
	
	return;
  }
  Yield();
  strcmp(&cmp, num[0], "testsendSF");
  if (cmp){
	// this is a send command
	// the first argument is the tid
	// the second argument is the message
	// create tasks
	if (iter_no < 3){
		uart_printf(CONSOLE, "Not enough arguments\r\n");
		return;
	}
	char args[1];

	int64_t N = atoi(num[1]);
	int64_t str_length = atoi(num[2]);
	Yield();
	// Init the reciever
	int tid = Create(1, k2_receiver);
	uart_printf(CONSOLE, "Created k2_receiver: %u\r\n", tid);
	Send(tid, num[1], str_length, args, 1); // send the N
	Send(tid, num[2], str_length, args, 1); // send the msglen


	// int tid = CreateArgs(5, k2_receiver, 1, args);
	//k2_sender(N, str_length);
	// send a message to the reciever
	// int tid = Create(5, k2_sender);


	char msg[str_length];
	for (int i = 0; i < str_length; i++)
	{
		msg[i] = 'A' + i % 26;
	}
	msg[str_length - 1] = 0;
	char msgreply[str_length];
	unsigned int start = get_timerLO();
	for (int i = 0; i < N; i++){
		
		//uart_printf(CONSOLE, "k2_sender: Sending message %u\r\n", i);
		Send(tid, msg, str_length, msgreply, str_length);
		//uart_printf(CONSOLE, "K2: Message sent, my reply is [%s] \r\n", msgreply);

	}
	//Send(tid, msg, str_length, msgreply, str_length);
	unsigned int end = get_timerLO();
	unsigned int avgtime = (end - start) / N;
	// print calculating variance
	//uart_printf(CONSOLE, "Calculating variance\r\n");
	// recreate the reciever task
	Yield();
	tid = Create(1, k2_receiver);
	// uart_printf(CONSOLE, "Created k2_receiver: %u\r\n", tid);
	Send(tid, num[1], str_length, args, 1); // send the N
	Send(tid, num[2], str_length, args, 1); // send the msglen
	// calculate the variance
	uint64_t variance = 0;
	// make the readings array 
	unsigned int readings[N];
	for (int i = 0; i < N; i++){
		unsigned int start = get_timerLO();
		Send(tid, msg, str_length, msgreply, str_length);
		unsigned int end = get_timerLO();
		unsigned int time = end - start;
		readings[i] = time;
	}
	for (int i = 0; i < N; i++){
		variance += (readings[i] - avgtime) * (readings[i] - avgtime);
	}
	variance = variance / N;

	uart_printf(CONSOLE, "Average Time taken for %u characters is %u ", str_length, avgtime);
	uart_printf(CONSOLE, "Variance is %u\r\n", variance);
	Yield();
	
	
	
	
	return;
  }
  Yield();
  strcmp(&cmp, num[0], "CreateArgs");
  if (cmp){
	  // this is a create args command
	  // the first argument is the priority
	  // the second argument is the function pointer
	  // the third argument is the number of arguments
	  // the fourth argument is the arguments
	  if (iter_no < 3){
		  uart_printf(CONSOLE, "Not enough arguments\r\n");
		  return;
	  }
	  int argsno = 2;
	  int64_t args[argsno];
	  args[0] = (int64_t)atoi(num[1]);
	  args[1] = (int64_t)atoi(num[2]);
	  uart_printf(CONSOLE, "argsno: %x\r\n", argsno);
	  uart_printf(CONSOLE, "args: %x\r\n", args);
	  int tid = CreateArgs(5, CreateArgsTest, argsno, args);
	  uart_printf(CONSOLE, "Created: %u\r\n", tid);
	  return;
  }
  Yield();
  // ad the adder test with predefined params of 1 - 10
  strcmp(&cmp, num[0], "adder");
  if (cmp){
	  // this is a create args command
	  // the first argument is the priority
	  // the second argument is the function pointer
	  // the third argument is the number of arguments
	  // the fourth argument is the arguments
	  int argsno = 10;
	  int64_t args[10];
	  /*
	  args[0] = (int64_t)atoi(num[1]);
	  args[1] = (int64_t)atoi(num[2]);
	  args[2] = (int64_t)atoi(num[3]);
	  args[3] = (int64_t)atoi(num[4]);
	  args[4] = (int64_t)atoi(num[5]);
	  args[5] = (int64_t)atoi(num[6]);
	  args[6] = (int64_t)atoi(num[7]);
	  args[7] = (int64_t)atoi(num[8]);
	  args[8] = (int64_t)atoi(num[9]);
	  args[9] = (int64_t)atoi(num[10]);
	  */
	 // the args are predetermined from 1 - 10
	  args[0] = 1;
	  args[1] = 2;
	  args[2] = 3;
	  args[3] = 4;
	  args[4] = 5;
	  args[5] = 6;
	  args[6] = 7;
	  args[7] = 8;
	  args[8] = 9;
	  args[9] = 10;

	  uart_printf(CONSOLE, "argsno: %x\r\n", argsno);
	  uart_printf(CONSOLE, "args: %x\r\n", args);
	  int tid = CreateArgs(5, adderTask, 10, args);
	  uart_printf(CONSOLE, "Created: %u\r\n", tid);
	  return;
  }
  Yield();
  uart_printf(CONSOLE, "Command not found\r\n");
}
void recieve_task(){
	RegisterAs(".");
	// first it would await for a message to arrive that would ultimatelly be a command to initialize the task
	Exit();
}

void Turnaround_tests(){
	/*
	R or S - indicating receiver first or sender first
	4, 64, or 256 - indicating message size
	the measured time per SRR operation in microseconds
	N = 100
	R is testsenderRF
	S is testsenderSF
	*/
	turnaround_test_parse_command("testsendRF 100 4");
	turnaround_test_parse_command("testsendRF 100 64");
	turnaround_test_parse_command("testsendRF 100 256");
	turnaround_test_parse_command("testsendSF 100 4");
	turnaround_test_parse_command("testsendSF 100 64");
	turnaround_test_parse_command("testsendSF 100 256");


}
