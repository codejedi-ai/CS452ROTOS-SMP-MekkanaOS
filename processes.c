#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"

#include "custstr.h"
#include "nameserver.h"
#include "gameserver.h"
#include "clockserver.h"

#include "systimer.h"
#include "tests/tc1tests.h"
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
void idle(){
	while(1){
		asm("WFI");
	}
	Exit();
}
void main(){
	// print in white font
	uart_printf(CONSOLE, "\033[37m");
	// register the k2
	RegisterAs("main");
	char *logo = "            ___     ___     ___     ___   __   __   ___     ___   \r\n    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n";
  	uart_printf(CONSOLE, "%s\r\n", logo);
  	uart_printf(CONSOLE, "Modified main to busywait \r\nHello World I am d273liu\r\n");
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
			
			if (tc1ExecuteCommands(command, num, command_part_count) != 2);
			else {
				uart_printf(CONSOLE, "ERROR: command is not valid command_part_count = %d\r\n", command_part_count);
			}
			
			// tc1(command);
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

void busyloop(){
	while(1) uart_printf(CONSOLE, "busyloop\r\n");
	uart_printf(CONSOLE, "busyloopexit\r\n");
	Exit();
}
#define UARTINTER 153
// new paradymn, run tests for each k# assignment (other than 3) before running the shell
void FirstUserTask() // First task as dictated in the reqs
{	// need to set the timer interrupt
	RegisterAs("FirstUserTask");
	int tid;
	int txic_tid = WhoIs("io_TXIC_MARKLIN_server");
	int main_pid = Create(-2, main);
	Exit();
}