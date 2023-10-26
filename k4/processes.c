#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "k2TimeTests.h"
#include "systimer.h"
#include "k2rps.h"
#include "clockserver.h"
#include "k3tests.h"
#include "asm.h"
#include "ioserver.h"
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


void main(){
	// register the k2
	RegisterAs("main");
	unsigned int counter=1;
	char command[50];
	int command_length = 0;
	command[0] = '\0';
	uart_printf(CONSOLE, "\r\nDARCY[%u]> ", counter++);
	char c = ' ';
	while (!(strcmp_ret(command, "quit"))) {
		while (!uart_getc_queue(CONSOLE)) {
			Yield();
		}
		c = uart_getc_modified(CONSOLE);
		if (c == '\r') {
			uart_printf(CONSOLE, "\r\n");
			// K2 commands
			// the parse char array changes the command 
      		
			char *num[100]; // array to store the numbers
			// int parse_char_arr(char *arr, char **num, int num_size)
			int command_part_count = parse_char_arr(command, num, 100);
			uart_printf(CONSOLE, "command = %s\r\n", command);
			for (int i = 0; i < command_part_count; i++){
				uart_printf(CONSOLE, "num[%d] = %s\r\n", i, num[i]);
			}
			if (k2ExecuteCommands(command, num, command_part_count) != -1);
			// if (k3ExecuteCommands(command, num, command_part_count) != -1);
			//if(k4ExecuteCommands(command, num, command_part_count) != 2);
			else {
				uart_printf(CONSOLE, "ERROR: command is not valid command_part_count = %d\r\n", command_part_count);
			}
			// K3 commands
			// The operating system is doomed to go to sleep or die after running the command

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

void idle(){
	while(1){
		asm("WFI");
		// uart_printf(CONSOLE, "idle: WFI <Print time here>\r\n");
		// uart_printf(CONSOLE, "idle: time = %u\r\n", time);
		uint32_t runtime = GetRuntime();
		uint32_t kernelrt = GetKernelRuntime();
		uart_printf(CONSOLE, "idle: runprecentage = %u \% \r\n", (100 * runtime) / kernelrt);
	}
	Exit();
}
void FirstUserTaskk3() // First task as dictated in the reqs
{	// need to set the timer interrupt
	uint32_t timer = get_timerLO();
	set_timerC3(timer + 10000);
	uart_printf(CONSOLE, "Timer C3: %u\r\n", get_timerC3());
	
	// We are assuming that FirstUserTask has a priority of 1
	// start gameserver
	RegisterAs("FirstUserTask");
  	int tid = KernelCreate(0, clock_notifier, 0);
	tid = KernelCreate(0, clock_server, 0);
	
	char clockproc1[8] = "cl10";
    uart_printf(CONSOLE, "%d\r\n", init_clock_proc(3, clockproc1, 10, 20));
	char clockproc2[8] = "cl23";
    uart_printf(CONSOLE, "%d\r\n", init_clock_proc(4, clockproc2, 23, 9));
	char clockproc3[8] = "cl33";
    uart_printf(CONSOLE, "%d\r\n", init_clock_proc(5, clockproc3, 33, 6));
	char clockproc4[8] = "cl71";
    uart_printf(CONSOLE, "%d \r\n", init_clock_proc(6, clockproc4, 71, 3));
	tid = Create(7, idle);
	uart_printf(CONSOLE, "idle: tid = %d\r\n", tid);
	// Create(2000, main);
	uart_printf(CONSOLE, "FirstUserTask: Started\r\n");
	Exit();
}
void busyloop(){
	while(1) uart_printf(CONSOLE, "busyloop\r\n");
	uart_printf(CONSOLE, "busyloopexit\r\n");
	Exit();
}
#define UARTINTER 153
void FirstUserTask() // First task as dictated in the reqs
{	// need to set the timer interrupt
	uint32_t timer = get_timerLO();
	set_timerC3(timer + 10000);
	// uart_printf(CONSOLE, "Timer C3: %u\r\n", get_timerC3());
	
	// We are assuming that FirstUserTask has a priority of 1
	// start gameserver
	// RegisterAs("FirstUserTask");
  	int tid = 0;
	tid = KernelCreate(0, clock_notifier, 0);
	tid = KernelCreate(0, clock_server, 0);
	
	tid = Create(-1, idle);
	
	int io_server_PID;
	io_server_PID = KernelCreate(0, io_notifier, 0);
	io_server_PID = KernelCreate(0, io_server, 0);
	uart_printf(CONSOLE, "io_server_PID = %d\r\n", io_server_PID);
	tid = Create(2000, main);
	Exit();
}