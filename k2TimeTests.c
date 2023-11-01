#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "gameserver.h"
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
// The name of process one and process two would be abbreviared as k2_sender and k2Receiver
void k2Receiver(){
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
	int64_t N = atoi_64(init_msg);
	Reply(mytid, "INIT", 4);

	// READ THE MSGLEN
	init_rec = Receive(&mytid, init_msg, 8);
	// int parse_char_arr(char *arr, char **num, int num_size);
	int64_t msglen = atoi_64(init_msg);
	Reply(mytid, "INIT", 4);


	mytid = MyTid();
	// uart_printf(CONSOLE, "k2Receiver: My tid is %u, N = %u, msglen = %u\r\n", mytid, N, msglen);

	int tid;
	char msg[msglen];
	char reply[msglen];
	for (int i = 0; i < msglen; i++)
	{
		reply[i] = 'A' + (i + 1) % 26;
	}
	reply[msglen - 1] = 0;

	// uart_printf(CONSOLE, "k2Receiver: My tid is %u\r\n", mytid);
	for (int i = 0; i < N; i++){
		// define the reply message here
		//uart_printf(CONSOLE, "k2Receiver: Waiting for message %u\r\n", i);
		// begin of the send
		// uart_printf(CONSOLE, "k2Receiver: Waiting for message %u\r\n", i);
		int recret = Receive(&tid, msg, msglen);
		// uart_printf(CONSOLE, "k2Receiver: Message recieved: [%s], recret = %d\r\n", msg, recret);
		int repret = Reply(tid, reply, msglen);
		// uart_printf(CONSOLE, "k2Receiver: Reply sent Repret = %d\r\n", repret);
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


void turnaroundTestParseCommand(char *arr) {
  
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
  /*  
  for (int j = 0; j < iter_no; j++) {
	uart_printf(CONSOLE, "num[%d] = %s\r\n", j, num[j]);
  }
  */
  int cmp;
  strcmp_inpace(&cmp, num[0], "yield");
  if (cmp){
	  Yield();
	  return;
  }
  Yield();
  //string compair
  strcmp_inpace(&cmp, num[0], "testsendRF");
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

	int64_t N = atoi_64(num[1]);
	int64_t str_length = atoi_64(num[2]);

	// Init the reciever
	int tid = Create(2, k2Receiver);
	//uart_printf(CONSOLE, "Created k2Receiver: %u\r\n", tid);
	Send(tid, num[1], str_length, args, 1); // send the N
	Send(tid, num[2], str_length, args, 1); // send the msglen
	// int tid = CreateArgs(5, k2Receiver, 1, args);
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
	
	tid = Create(2, k2Receiver);
	// uart_printf(CONSOLE, "Created k2Receiver: %u\r\n", tid);
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
  strcmp_inpace(&cmp, num[0], "testsendSF");
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

	int64_t N = atoi_64(num[1]);
	int64_t str_length = atoi_64(num[2]);
	Yield();
	// Init the reciever
	int tid = Create(1, k2Receiver);
	//uart_printf(CONSOLE, "Created k2Receiver: %u\r\n", tid);
	Send(tid, num[1], str_length, args, 1); // send the N
	Send(tid, num[2], str_length, args, 1); // send the msglen


	// int tid = CreateArgs(5, k2Receiver, 1, args);
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
	tid = Create(1, k2Receiver);
	// uart_printf(CONSOLE, "Created k2Receiver: %u\r\n", tid);
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
  strcmp_inpace(&cmp, num[0], "CreateArgs");
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
	  args[0] = (int64_t)atoi_64(num[1]);
	  args[1] = (int64_t)atoi_64(num[2]);
	  uart_printf(CONSOLE, "argsno: %x\r\n", argsno);
	  uart_printf(CONSOLE, "args: %x\r\n", args);
	  int tid = CreateArgs(5, CreateArgsTest, argsno, args);
	  uart_printf(CONSOLE, "Created: %u\r\n", tid);
	  return;
  }
  Yield();
  // ad the adder test with predefined params of 1 - 10
  strcmp_inpace(&cmp, num[0], "adder");
  if (cmp){
	  // this is a create args command
	  // the first argument is the priority
	  // the second argument is the function pointer
	  // the third argument is the number of arguments
	  // the fourth argument is the arguments
	  int argsno = 10;
	  int64_t args[10];
	  /*
	  args[0] = (int64_t)atoi_64(num[1]);
	  args[1] = (int64_t)atoi_64(num[2]);
	  args[2] = (int64_t)atoi_64(num[3]);
	  args[3] = (int64_t)atoi_64(num[4]);
	  args[4] = (int64_t)atoi_64(num[5]);
	  args[5] = (int64_t)atoi_64(num[6]);
	  args[6] = (int64_t)atoi_64(num[7]);
	  args[7] = (int64_t)atoi_64(num[8]);
	  args[8] = (int64_t)atoi_64(num[9]);
	  args[9] = (int64_t)atoi_64(num[10]);
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
void recieveTask(){
	RegisterAs(".");
	// first it would await for a message to arrive that would ultimatelly be a command to initialize the task
	Exit();
}

void turnaroundTests(){
	/*
	R or S - indicating receiver first or sender first
	4, 64, or 256 - indicating message size
	the measured time per SRR operation in microseconds
	N = 100
	R is testsenderRF
	S is testsenderSF
	*/
	turnaroundTestParseCommand("testsendRF 100 4");
	turnaroundTestParseCommand("testsendRF 100 64");
	turnaroundTestParseCommand("testsendRF 100 256");
	turnaroundTestParseCommand("testsendSF 100 4");
	turnaroundTestParseCommand("testsendSF 100 64");
	turnaroundTestParseCommand("testsendSF 100 256");


}



void testNameserver(){
	char* myname = "testNameserver";
	// set coulor to green
	uart_printf(CONSOLE, "\033[32m");
	uart_printf(CONSOLE, "testNameserver: My name is %s\r\n", myname);
	// set coulor to white
	
	//call register
	uart_printf(CONSOLE, "testNameserver: Registering my name\r\n");
	uart_printf(CONSOLE, "\033[37m");
	RegisterAs(myname);
	//call whois
	// set coulor to green
	uart_printf(CONSOLE, "\033[32m");
	uart_printf(CONSOLE, "testNameserver: Calling whois on myself\r\n");
	// set coulor to white
	uart_printf(CONSOLE, "\033[37m");
	int tid = WhoIs(myname);
	// set coulor to green
	uart_printf(CONSOLE, "\033[32m");
	uart_printf(CONSOLE, "testNameserver: tid = %u\r\n", tid);
	// set coulor to white
	uart_printf(CONSOLE, "\033[37m");
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
	uart_printf(CONSOLE, "k2Receiver: My tid is %u\r\n", mytid);
	int recret = Receive(&tid, msg, msglen);
	uart_printf(CONSOLE, "k2Receiver: Message recieved: [%s], recret = %d\r\n", msg, recret);
	for (int i = 0; i < msglen; i++)
	{
		uart_putc(CONSOLE, reply[i]);
	}
	uart_printf(CONSOLE, "\r\n");

	// define the reply message here
	int repret = Reply(tid, reply, msglen);
	uart_printf(CONSOLE, "k2Receiver: Reply sent Repret = %d\r\n", repret);
	Exit();	
}
void rockPlayer(){
	int tid = MyTid();
	signup();
	// play rock paper scissors
	int play_ret = play("rock");
	uart_printf(CONSOLE, "Play %u, player:%d have: ",1, tid);
	uart_putc(CONSOLE, (char)play_ret);
	uart_printf(CONSOLE, "\r\n");
	quit();
	Exit();
}


int k2ExecuteCommands(char *command, char **num, int command_part_count){
	int Priority = MyPriority();

	if(strcmp_ret(num[0],"k2pm")){
		Yield();
		uart_printf(CONSOLE, "K2: STARTING SEND/RECIEVE/REPLY PERFORMANCE TESTS: %u\r\n");
		turnaroundTests(); // should this be a process or just some tests?
	} else if (strcmp_ret(num[0],"k2rps")){
		Yield();
		if (strcmp_ret(num[1], "start")){
		int tid = Create(Priority, gameserver);
			uart_printf(CONSOLE,"gameserver Created: %u\r\n", tid);
		// uart_printf(CONSOLE, "K2: STARTING Rock Paper Scissors TESTS: %u\r\n");
		} else if (WhoIs("gameserver") == NUMPROCS){
			uart_printf(CONSOLE, "K2: ERROR: gameserver is not running, run k2rps start to run the server\r\n");
			return 0;
		} else if (strcmp_ret(num[1], "shutdown")){
		uart_printf(CONSOLE, "K2: SHUTTING DOWN Rock Paper Scissors TESTS: %u\r\n");
		RPCShutdown();
		} else if (strcmp_ret(num[1], "create")){
		// this is the create command that creates the players
		// create N 0 rock player
		// create N 1 paper player
		// create N 2 scissors player
		// create N 3 random player
		if (command_part_count != 4){
			uart_printf(CONSOLE, "K2: ERROR: create command is not valid k2RPS start N <type>\r\n");
			return 0;
		}
		uint64_t N = atoi_64(num[2]);
		if (N > 10){
			uart_printf(CONSOLE, "K2: ERROR: N is too large: %u\r\n", N);
			return;
		}
		
		uint64_t type;
		if (strcmp_ret(num[3], "rock")){
			type = 0;
		} else if (strcmp_ret(num[3], "paper")){
			type = 1;
		} else if (strcmp_ret(num[3], "scissors")){
			type = 2;
		} else if (strcmp_ret(num[3], "random")){
			type = 3;
		} else {
			uart_printf(CONSOLE, "K2: ERROR: type is not valid: %s\r\n", num[3]);
			return 0;
		}
		initPlayer(N, type, Priority - 1);
		
		} else if(strcmp_ret(num[1], "play")) {
			char play_ret = play(num[2]);
			if (play_ret == 'E'){
				uart_printf(CONSOLE, "K2: ERROR: k2RPS not signed up, please sign up to join game\r\n");
				return 0;
			}
			
			uart_printf(CONSOLE, "You have: ");
			uart_putc(CONSOLE, (char)play_ret);
			uart_printf(CONSOLE, "\r\n");
		} else if(strcmp_ret(num[1], "signup")){
			signup();
		} else if(strcmp_ret(num[1], "quit")){
			quit();
		}else {
		uart_printf(CONSOLE, "K2: ERROR: k2RPS command is not valid\r\n");
		}
	


	
      }
	  // the command is not found
	  else {
		  return -1;
	  }
	  return 1;
}
void k2FirstUserTask() // First task as dictated in the reqs
{
	// We are assuming that FirstUserTask has a priority of 1
	// start gameserver
	RegisterAs("FirstUserTask");
	int gameserver_tid = Create(2, gameserver);
	// int gameserver_tid2 = Create(1, rockPlayer);
	// int gameserver_tid3 = Create(1, rockPlayer);
	initPlayer(1, 0, 2);
	initPlayer(1, 1, 2);
	RPCShutdown();
	Create(100, main);
	Exit();
}