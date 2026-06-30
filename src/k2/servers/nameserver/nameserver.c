#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#define DEBUG 0
void helper_parsestring( char *retarr, int size, char * str, int part) {
  int i = 1, j = 0;
  retarr[0] = 0;
  while (*str) {
    if (*str == ' ') {
      i++;
    }else if (part == i){
      if (j >= size - 1) {
        break;
      }
      retarr[j++] = *str;
      retarr[j] = 0;
    }
    str++;
  }
  
}
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
	char pid_names[NUMPROCS][50];
	int tid;
	char msg[50];
	int msglen = 49;
	for (int i = 0; i < NUMPROCS; i++)
	{
		pid_names[i][0] = 0;
	}
	cust_strcpy(pid_names[1], 50, "nameserver", 50);
	cust_strcpy(pid_names[0], 50, "kernel", 50);
	while (1)
	{
		Receive(&tid, msg, msglen);

		char command[50];
		char name[50];
		command[0] = 0;
		name[0] = 0;

		helper_parsestring(command, 50, msg, 1);
		helper_parsestring(name, 50 , msg, 2);
		// uart_printf(CONSOLE, "DBG recv: msg='%s' -> cmd='%s' name='%s'\r\n", msg, command, name);

		if (strcmp_ret(command, "REGISTER", 0)){
			cust_strcpy(pid_names[tid], 50, name, 50);
			// uart_printf(CONSOLE, "DBG REGISTER: tid=%d registered as '%s'\r\n", tid, pid_names[tid]);
			int repret = 0;
			Reply(tid, &repret, 0);
		} else if (strcmp_ret(command, "WHOIS", 0)){
			int ret = 0;
			// uart_printf(CONSOLE, "DBG WHOIS from tid=%d: searching for '%s' (pid_names[3]='%s')\r\n", tid, name, pid_names[3]);
			while (ret < NUMPROCS && !strcmp_ret(pid_names[ret], name, 0)){
				ret++;
			}
			if(ret >= NUMPROCS){
				// uart_printf(CONSOLE, "  -> Not found (checked all, returned -1)\r\n");
				ret = -1;
			} else {
				// uart_printf(CONSOLE, "  -> Found at %d\r\n", ret);
			}
			Reply(tid, &ret, 4);
		} else if (strcmp_ret(command, "DEREGISTER", 0)){
			pid_names[tid][0] = 0;
			int repret = 0;
			Reply(tid, &repret, 0);
		}
	}
	Exit();
}
// must return -1 if not found
int RegisterAs(const char *name){
	int rep = 0;
	char sendmsg[50] = "REGISTER ";
	int msgsz = strcat_cust((char* )sendmsg, name);
	msgsz++;  // Include null terminator
	Send(1, sendmsg, msgsz, (char *)&rep, 4);
	return rep;
}
int Deregister(){
	int tid = MyTid();
	char rep[50];
	char sendmsg[50] = "DEREGISTER ";
	char tid_str[10] = "";
	i2a(tid, tid_str);
	int msgsz = strcat_cust((char* )sendmsg, (char *)tid_str);
	msgsz++;  // Include null terminator
	return Send(1, sendmsg, msgsz, rep, 50);
}
int WhoIs(const char *name){
	int rep = 0;
	char sendmsg[50] = "WHOIS ";
	int msgsz = strcat_cust((char* )sendmsg, name);
	msgsz++;  // Include null terminator
	Send(1, sendmsg, msgsz, (char *)&rep, 4);
	return rep;
}