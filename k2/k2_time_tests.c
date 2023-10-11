#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "processes.h"
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
	int msglen = 10;
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
	int N = str_to_int(init_msg);
	Reply(mytid, "INIT", 4);

	// READ THE MSGLEN
	init_rec = Receive(&mytid, init_msg, 8);
	// int parse_char_arr(char *arr, char **num, int num_size);
	msglen = str_to_int(init_msg);
	Reply(mytid, "INIT", 4);


	mytid = MyTid();
	uart_printf(CONSOLE, "k2_receiver: My tid is %u, N = %u, msglen = %u\r\n", mytid, N, msglen);

	int tid;
	char msg[msglen];
	char reply[msglen];
	for (int i = 0; i < msglen; i++)
	{
		reply[i] = 'A' + (i + 1) % 26;
	}
	reply[msglen - 1] = 0;

	uart_printf(CONSOLE, "k2_receiver: My tid is %u\r\n", mytid);
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


void parse_command(char *arr) {
  
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

	int N = str_to_int(num[1]);
	int str_length = str_to_int(num[2]);

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

	int N = str_to_int(num[1]);
	int str_length = str_to_int(num[2]);
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
	uart_printf(CONSOLE, "Calculating variance\r\n");
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

	uart_printf(CONSOLE, "Average Time taken for %d characters is %u ", str_length, avgtime);
	uart_printf(CONSOLE, "Variance is %u\r\n", variance);
	Yield();
	
	
	
	
	return;
  }
  Yield();
  strcmp(&cmp, num[0], "first_task");
  if (cmp){
	  // this is a recieve command
	  // the first argument is the tid
	  // the second argument is the message
	  // create tasks
	  if (iter_no < 1){
		  uart_printf(CONSOLE, "Not enough arguments\r\n");
		  return;
	  }
	  int tid = Create(5, first_task);
	  // int tid = CreateArgs(5, k2_sender, 1, args);
	  //k2_receiver();
	  uart_printf(CONSOLE, "Created: %u\r\n", tid);
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
	  args[0] = (int64_t)str_to_int(num[1]);
	  args[1] = (int64_t)str_to_int(num[2]);
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
	  args[0] = (int64_t)str_to_int(num[1]);
	  args[1] = (int64_t)str_to_int(num[2]);
	  args[2] = (int64_t)str_to_int(num[3]);
	  args[3] = (int64_t)str_to_int(num[4]);
	  args[4] = (int64_t)str_to_int(num[5]);
	  args[5] = (int64_t)str_to_int(num[6]);
	  args[6] = (int64_t)str_to_int(num[7]);
	  args[7] = (int64_t)str_to_int(num[8]);
	  args[8] = (int64_t)str_to_int(num[9]);
	  args[9] = (int64_t)str_to_int(num[10]);
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
			parse_command(command);
			command_length = 0;
			command[0] = '\0';
			Yield();
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