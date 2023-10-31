#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "k2rps.h"
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
void rock_player(){
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


void execute_commands(char *command){
	int Priority = MyPriority();
  		char *num[100]; // array to store the numbers
			// int parse_char_arr(char *arr, char **num, int num_size)
			int command_part_count = parse_char_arr(command, num, 100);
			if(strcmp_ret(num[0],"k2pm")){
        Yield();
        uart_printf(CONSOLE, "K2: STARTING SEND/RECIEVE/REPLY PERFORMANCE TESTS: %u\r\n");
				Turnaround_tests(); // should this be a process or just some tests?
			} else if (strcmp_ret(num[0],"k2rps")){
        Yield();
        if (strcmp_ret(num[1], "start")){
          int tid = Create(10, gameserver);
	        uart_printf(CONSOLE,"gameserver Created: %u\r\n", tid);
           // uart_printf(CONSOLE, "K2: STARTING Rock Paper Scissors TESTS: %u\r\n");
        } else if (WhoIs("gameserver") == NUMPROCS){
            uart_printf(CONSOLE, "K2: ERROR: gameserver is not running, run k2rps start to run the server\r\n");
            return;
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
            return;
          }
          uint64_t N = atoi(num[2]);
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
            return;
          }
          initPlayer(N, type, Priority + 1);
          
        } else if(strcmp_ret(num[1], "play")) {
			char play_ret = play(num[2]);
			if (play_ret == 'E'){
				uart_printf(CONSOLE, "K2: ERROR: k2RPS not signed up, please sign up to join game\r\n");
				return;
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
		  uart_printf(CONSOLE, "ERROR: command is not valid\r\n");
	  }
}
void main(){
	// register the k2
	RegisterAs("main");
	unsigned int counter=1;
	char command[50];
	int command_length = 0;
	command[0] = '\0';
	uart_printf(CONSOLE, "DARCY[%u]> ", counter++);
	char c = ' ';
	while (!(strcmp_ret(command, "quit"))) {
		while (!uart_getc_queue(CONSOLE)) {
			Yield();
		}
		c = uart_getc_modified(CONSOLE);
		if (c == '\r') {
			uart_printf(CONSOLE, "\r\n");
      		execute_commands(command);
			command_length = 0;
			command[0] = '\0';
			Yield();
			uart_printf(CONSOLE, "\r\nDARCY[%u]> ", counter++);
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
	uart_printf(CONSOLE, "\r\n");
	
	// print white font
	uart_printf(CONSOLE, "\033[37m");
	Exit();
}
void first_task() // First task as dictated in the reqs
{
	// We are assuming that first_task has a priority of 1
	// start gameserver
	RegisterAs("first_task");
	int gameserver_tid = Create(2, gameserver);
	// int gameserver_tid2 = Create(1, rock_player);
	// int gameserver_tid3 = Create(1, rock_player);
	initPlayer(1, 0, 2);
	initPlayer(1, 1, 2);
	RPCShutdown();
	Create(1, main);
	Exit();
}