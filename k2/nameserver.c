#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
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
void nameserver(){
	char* pid_names[NUMPROCS][50];
	int tid;
	char msg[50];
	int msglen = 49;
	for (int i = 0; i < NUMPROCS; i++)
	{
		pid_names[i][0] = 0;
	}
	strcpy(pid_names[0], 50, "nameserver", 50);
	while (1)
	{

		int recret = Receive(&tid, msg, msglen);
		// strflush(msg, 50);

		char command[50];
		char name[50];


		parsestring(command, 50, msg, 1);
		parsestring(name, 50 , msg, 2);


		// now print the compair
		char *command_cand = "REGISTER";
		int cmp;
		// strcmp(&cmp, command, command_cand);
		cmp = strcmp_ret(command, command_cand);

		if (cmp){
			strcpy(pid_names[tid], 50, name, 50);
			// this registers a PID with a name
			// change font to blue


			// change font to white

			int repret = Reply(tid, "PID Registered", 25);
		}
		command_cand = "WHOIS";
		strcmp(&cmp, command, command_cand);

		if (cmp){

			// this registers a PID with a name
			int ret = 0;
			while (ret < NUMPROCS)
			{

				if (strcmp_ret(pid_names[ret], name))
				{
					break;
				}
				ret++;
			}
			// unsigned int to ascii string
			char bf[10] = "";
			ui2a(ret, 10, bf);

			int repret = Reply(tid, bf, 25);
		}
		command_cand = "DEREGISTER";
		strcmp(&cmp, command, command_cand);
		if (cmp){
			// this registers a PID with a name
			// print the deregistered in red
			uart_printf(CONSOLE, "\033[31m");
			uart_printf(CONSOLE, "DEREGISTERD PID: %u, Name: %s\r\n", tid, pid_names[tid]);
			uart_printf(CONSOLE, "\033[37m");
			pid_names[tid][0] = 0;
			int repret = Reply(tid, "PID Deregistered", 25);
		}
		// print the registered table in green
		uart_printf(CONSOLE, "\033[32m");
		uart_printf(CONSOLE, "Registered Table:\r\n");
		for (int i = 0; i < NUMPROCS; i++)
		{
			
			uart_printf(CONSOLE, "PID: %u, Name: %s\r\n", i, pid_names[i]);
		}
		
		uart_printf(CONSOLE, "\033[37m");
	}
	Exit();
}
int RegisterAs(const char *name){
	char rep[50];
	char sendmsg[50] = "REGISTER ";
	//// strflush(command_cand, 6);
	int msgsz = 50;
	//msgsz = stringconcat((char* )sendmsg, command_cand);
	//// strflush(sendmsg, msgsz);
	msgsz = stringconcat((char* )sendmsg, name);
	//// strflush(sendmsg, msgsz);
	Send(1, sendmsg, 50, rep, 50);
	return 0;
}
int Deregister(){
	int tid = MyTid();
	char rep[50];
	char sendmsg[50] = "DEREGISTER ";
	char tid_str[10] = "";
	i2a(tid, tid_str);
	int msgsz = 50;
	msgsz = stringconcat((char* )sendmsg, (char *)tid_str);
	// strflush(sendmsg, msgsz);
	return Send(1, sendmsg, 50, rep, 50);
}
int WhoIs(const char *name){
	char rep[50];
	char sendmsg[50] = "WHOIS ";

	//// strflush(command_cand, 6);
	int msgsz = 50;
	// msgsz = stringconcat((char* )sendmsg, command_cand);
	//// strflush(sendmsg, msgsz);
	msgsz = stringconcat((char* )sendmsg, name);
	// strflush(sendmsg, msgsz);
	
	//int ret_code = Send(tid, msg, msglen, msgreply, 25);
	Send(1, sendmsg, 50, rep, 50);
	return str_to_int(rep);
}