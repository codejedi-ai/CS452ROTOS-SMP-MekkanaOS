#include "syscall.h"
#include "processes.h"
#include "asm.h"
#include "rpi.h"
#include "util.h"

static void *STACKSTART;
// This is the PID of the currentlly running process
static int PID = 0;




// static const int NUMPROCS = 20; // Deprecated
static struct process PROCS[NUMPROCS];
int heap_size = 0;
static struct state READY_QUEUE[NUMPROCS];
static struct state BLOCKED_QUEUE[NUMPROCS];
#define DEBUG 2
// it is time to turn READY_QUEUE into a heap
// enqueing on a heap is O(log(n))
// first you 

// scrSchedule(pid, priority, ready)
// This is an enqueue funciton in which it adds a process to the READY_QUEUE 
void scrSchedule(int pid, int priority, int ready)
{
	struct state currItem = {pid, priority, ready};
	struct state nextItem;
	int insert = 0;
	for (int i = 0; i < NUMPROCS; i++) {
		if (READY_QUEUE[i].pid == 0) {
			READY_QUEUE[i] = currItem;
			return 0;	
		}
		else if (READY_QUEUE[i].priority < priority) {
			insert = 1;
		}
		if (insert) {
			nextItem = READY_QUEUE[i];
			READY_QUEUE[i] = currItem;
			currItem = nextItem;
		}
	}
	return 0;
}
// scrSchedule(pid, priority, ready)
// This is an enqueue funciton in which it adds a process to the READY_QUEUE 
void queue_unblock(int pid, int priority, int ready)
{
	struct state currItem = {pid, priority, ready};
	struct state nextItem;
	int insert = 0;
	for (int i = 0; i < NUMPROCS; i++) {
		if (READY_QUEUE[i].pid == 0) {
			// reached the end of the queue so ret
			return 0;	
		}
		else if (READY_QUEUE[i].priority == priority && READY_QUEUE[i].pid == pid) {
			READY_QUEUE[i].ready = ready;
		}
	}
	return 0;
}
int scrPick()
{
	int pid = -1;
	int bump = 0;
	struct state emptyItem = {0, 0, 0};
	
	for (int i = 0; i < NUMPROCS; i++) {
		if (bump) {
			READY_QUEUE[i - 1] = READY_QUEUE[i];
		}
		else if (READY_QUEUE[i].pid > 0 && READY_QUEUE[i].ready == READY) {
			pid = READY_QUEUE[i].pid;
			bump = 1;
		}
		
	}
	if (bump) {READY_QUEUE[NUMPROCS - 1] = emptyItem;}
	return pid;
}



void InitSys(void* reg)
{	// For some reason, normal init to 0 just.. doesn't work?
	STACKSTART = reg;
	PID = 0;
	for (int idx = 0; idx < NUMPROCS; idx++) {
		PROCS[idx].stackpointer = NULL;
		PROCS[idx].pcpointer = NULL;
		PROCS[idx].pstate = 0;
		PROCS[idx].parentpid = 0;
		PROCS[idx].priority = 0;
		for (int jdx = 0; jdx < 31; jdx++) {
			PROCS[idx].registervalues[jdx] = 10 + jdx;
		
		}
		
		READY_QUEUE[idx].pid = 0;
		READY_QUEUE[idx].ready = 0;
		READY_QUEUE[idx].priority = 0;
		
	}
	return 0;
}
// This is the function that is called when a syscall is made
// This is the contextswitch

void Handle(void* sp) // A helper function to pull some c variables into assembly
{
	// We just arrived here, there is stuff on the stack that I do not want to deal with
	int p = PID - 1;
	
	#if DEBUG == 1
	uart_printf(CONSOLE, "Handling %x %x %x %x %x\r\n", sp, &PROCS[p].registervalues[0], &PROCS[p].pcpointer, &PROCS[p].stackpointer, &PROCS[p].pstate);
	#endif
	
	Save(sp, &PROCS[p].registervalues[0], &PROCS[p].pcpointer, &PROCS[p].stackpointer, &PROCS[p].pstate);
	return 0; // return should NEVER be called here, as it is a one-way path
}

int strcpy(char *dest, int lenDes, char *src, int lenSrc){
	int i = 0;
	while (*src) {
		if (i >= lenSrc) {break;}
		if (i >= lenDes) {break;}
		*dest = *src;
		dest++;
		src++;
		i++;
	}
	*dest = 0;
	return i;
}
int8_t dead(int8_t p){
	return (PROCS[p].stackpointer == NULL && PROCS[p].pcpointer == NULL);
}
// Each parameter is now stored in the registers
void send_helper(){
	# if DEBUG == 2
	// print the function called
	uart_printf(CONSOLE, "===============\r\n Send Helper Called:\r\n");
	# endif
	// Debug
	int p = PID - 1;
	int tid = PROCS[p].registervalues[0];
	char *msg = PROCS[p].registervalues[1];
	int msglen = PROCS[p].registervalues[2];
	char *reply = PROCS[p].registervalues[3];
	int replylen = PROCS[p].registervalues[4];

	// This puts the message into the messageDS of the target task
	PROCS[p].message_sent.tid = tid;
	PROCS[p].message_sent.msg = msg;
	PROCS[p].message_sent.msglen = msglen;
	PROCS[p].message_sent.reply = reply;
	PROCS[p].message_sent.replylen = replylen;
	PROCS[p].waiting_reply = 1;
	// This is the target task, if it is waiting_send then we need ot remove the waiting send and unblock the task
	p = tid - 1;
	int tail = PROCS[p].waiting_recieve_tail;
	PROCS[p].message_recieved[tail].tid = PID; // This is the tricky part for the recieved it should be the sender's pid
	PROCS[p].message_recieved[tail].msg = msg;
	PROCS[p].message_recieved[tail].msglen = msglen;
	PROCS[p].message_recieved[tail].reply = reply;
	PROCS[p].message_recieved[tail].replylen = replylen;
	uart_printf(CONSOLE, "reply addr is %x\r\n", reply);
	PROCS[p].waiting_recieve_tail++;
	PROCS[p].waiting_recieve_tail %= 50;
	
	# if DEBUG == 2
	// print the function called
	uart_printf(CONSOLE, "===============\r\n Completed adding message to the queue:\r\n");
	// print all the params
	uart_printf(CONSOLE, "TID is %u\r\n", tid);
	uart_printf(CONSOLE, "MSG is %s\r\n", msg);
	uart_printf(CONSOLE, "MSGLEN is %u\r\n", msglen);
	uart_printf(CONSOLE, "REPLY is %s\r\n", reply);
	uart_printf(CONSOLE, "REPLYLEN is %u\r\n ============== \r\n", replylen);
	# endif
	if (PROCS[p].waiting_send == 1){
		// The task is waiting for a message
		// unblock the task
		p = tid - 1;
		# if DEBUG == 2
		// print therefore unblocked
		uart_printf(CONSOLE, "===============\r\n RECIEVE HELPER FOR %u by Send Helper Unblocked:\r\n", tid);
		# endif

		recieve_helper(tid);
	}
	// At this point we need to wake up the message processing task
	
}
// It assumes that the messageDS is not empty
// recieve takes a message from the mailbox and returns the message inplace
void recieve_helper(int PID){
	int head = PROCS[PID - 1].waiting_recieve_head;
	int tail = PROCS[PID - 1].waiting_recieve_tail;
	# if DEBUG
	// print PID
	uart_printf(CONSOLE, "PID is %u\r\n", PID);
	uart_printf(CONSOLE, "head is %u, tail is %u\r\n", head, tail);
	# endif
	
	int *tid =  PROCS[PID - 1].registervalues[0]; // This is a memory address for the TID
	char *msg = PROCS[PID - 1].registervalues[1]; // this is another memory address for the message
	int msglen = PROCS[PID - 1].registervalues[2];
	# if DEBUG == 2
	// print therefore blocked
	
	// print the value of the tid poitnter msg pointer and the msglen pointer
	uart_printf(CONSOLE, "p = %d\r\n", PID - 1);
	uart_printf(CONSOLE, "TID is %x\r\n", tid);
	uart_printf(CONSOLE, "MSG is %x\r\n", msg);
	uart_printf(CONSOLE, "MSGLEN is %u\r\n ===============\r\n ", msglen);

	# endif
	if (head == tail){
		uart_printf(CONSOLE, "===============\r\n Recieve Helper Blocked:\r\n");
		// The messageDS is empty
		// Block the task
		// it is replying to a not reply blocked task

		PROCS[PID - 1].waiting_send = 1;
		return;
	}

	// unblock the task I really do not know how to unblock the task. If it was just blankly unblocked it would just return 
	// and keep running with no message
	
	// HOWEVER THE TASK IS STILL BLOCKED ONE MUST REPLY TO THE MESSAGE
	# if DEBUG == 2
	// Print the function called
	uart_printf(CONSOLE, "===============\r\n Recieve Helper Called:\r\n");
	# endif
	int p = PID - 1;
	// Those are the returning variables. The memories needed to be written when the recieve function returns
	// THE RECIEVING TASK IS READING INTO THE MEMORY BUFFER OF THE SENDING TASK
	// *tid is a pointer to the memory address of the TID




	
	// First need to access the mailbox
	// The mailbox is the messageDS of the process
	// The mailbox is a circular READY_QUEUE

	int  curread_message_length = PROCS[p].message_recieved[head].msglen;
	char *curread_msg = PROCS[p].message_recieved[head].msg;
	int sender_tid = PROCS[p].message_recieved[head].tid;
	*tid = sender_tid;
	// msg is the destination curread_msg is the source
	msglen = strcpy(msg, msglen, curread_msg, curread_message_length);
	// DEBUG
	# if DEBUG == 2
	// Print the recieved message
	uart_printf(CONSOLE, "head is %u, tail is %u\r\n", head, tail);
	uart_printf(CONSOLE, "curread_msg is %s\r\n", curread_msg);
	uart_printf(CONSOLE, "curread_message_length is %u\r\n", curread_message_length);
	uart_printf(CONSOLE, "sender_tid TID is %u\r\n", sender_tid);
	uart_printf(CONSOLE, "msg is %s\r\n", msg);
	uart_printf(CONSOLE, "MSGLEN is %u\r\n ===============\r\n ", curread_message_length);
	# endif
	
	// update the head

	# if DEBUG == 2
	// Print the recieved message
	uart_printf(CONSOLE, "TID is %u\r\n", sender_tid);
	uart_printf(CONSOLE, "curread_msg is %s\r\n", curread_msg);
	uart_printf(CONSOLE, "MSGLEN is %u\r\n ===============\r\n ", msglen);
	# endif
	PROCS[p].waiting_recieve_head++;
	PROCS[p].waiting_recieve_head %= 50;
	queue_unblock(PID, PROCS[PID - 1].priority, READY);
	// return msglen;
	PROCS[PID - 1].registervalues[0] = msglen;
}
void reply_helper(){
	# if DEBUG == 2
	uart_printf(CONSOLE, "===============\r\n Reply Helper Called:\r\n");
	# endif
	// replies to the PID
	int p = PID - 1;
	int tid = PROCS[p].registervalues[0];
	char *reply = (char *)PROCS[p].registervalues[1];
	int replylen = PROCS[p].registervalues[2];
	char *reply_buffer = PROCS[tid - 1].message_sent.reply;
	int reply_buffer_len = PROCS[tid - 1].message_sent.replylen;
	// Have the kernel copy the reply into the messageDS of the target task
	# if DEBUG == 2
	// Print the recieved message
	uart_printf(CONSOLE, "TID is %u\r\n", tid);
	uart_printf(CONSOLE, "REPLY is %s\r\n", reply);
	uart_printf(CONSOLE, "REPLYLEN is %u\r\n ===============\r\n ", replylen);
	uart_printf(CONSOLE, "replying to PID is %u, reply PID is %u\r\n", tid , PID );
	uart_printf(CONSOLE, "reply_buffer is %x, *reply is %x\r\n", reply_buffer, reply);
	# endif
	// PROCS[tid - 1].message_sent.reply[replylen] = 0;


	// uart_printf(CONSOLE, "*reply is %c\r\n", *reply);
	int i = 0;
	replylen = strcpy(reply_buffer, reply_buffer_len, reply, replylen);
	// 
	// now unblock the target task
	queue_unblock(tid, PROCS[tid - 1].priority, READY);
	// return a 0 for the send function
	PROCS[tid - 1].registervalues[0] = replylen;
	PROCS[tid - 1].waiting_reply = 0;
	// return for the reply function
	PROCS[PID - 1].registervalues[0] = replylen;
	
}

void Exception(uint64_t esr_el1)
{
	#if DEBUG == 1
	uart_printf(CONSOLE, "ESR is %x\n\r", esr_el1); // DEBUG PRINT
	#endif
	// PID is the currentlly running process
	int p = PID - 1;
	if (esr_el1 >> 24 == 86) { // an svc call has occured!
		int i = esr_el1 % 0x1000000;
		
		#if DEBUG == 1
		// print the running PID
		// uart_printf(CONSOLE, "PID is %u ", PID); // DEBUG PRINT
		// uart_printf(CONSOLE, "Case is %x\n\r", i); // DEBUG PRINT
		#endif
		
		switch (i) {
		
		case 0: // Exit
			Kill(p);
			break;
		case 1: // Yield
			scrSchedule(PID, PROCS[p].priority, READY);
			break;
		case 2: // Create
			scrSchedule(PID, PROCS[p].priority, READY);
			int ret = KernelCreate(PROCS[p].registervalues[0], PROCS[p].registervalues[1], p + 1);
			PROCS[p].registervalues[0] = ret;
			break;
		case 3: // mytid
			scrSchedule(PID, PROCS[p].priority, READY);
			PROCS[p].registervalues[0] = PROCS[p].pid;
			break;
		case 4: // parenttid
			// uart_printf(CONSOLE, "PPID is %u\n\r", PROCS[p].parentpid); // DEBUG PRINT
		
			scrSchedule(PID, PROCS[p].priority, READY);
			PROCS[p].registervalues[0] = PROCS[p].parentpid;
			break;
		case 5: // send blocks and unblocks other tasks
			// This unblocks the recieving task
			int tid_dest = PROCS[p].registervalues[0];
			
			if (dead(tid_dest - 1)){
				# if DEBUG == 2
				// print apparentlly tid_dest is dead
				uart_printf(CONSOLE, "%u is dead\r\n", tid_dest);
				# endif
				// The destination task does not exist
				scrSchedule(PID, PROCS[p].priority, READY);
				PROCS[p].registervalues[0] = -1;
			}
			
			else
			{
				// the destination does exist
				// Alright unblock the recieving task if it is blocked and expecting message
				// Blocks until a reply is generated
				scrSchedule(PID, PROCS[p].priority, BLOCKED);
				//PROCS[p].registervalues[0] = 
				send_helper();
			}
			// however there is another case in which the task unblocks
			break;
		case 6: // recieve blocks

			// There is a message in the mailbox
			scrSchedule(PID, PROCS[p].priority, BLOCKED);
			//PROCS[p].registervalues[0] = 
			recieve_helper(PID);
			
			break;
		case 7: // reply
			scrSchedule(PID, PROCS[p].priority, READY);
			tid_dest = PROCS[p].registervalues[0];
			if(dead(tid_dest - 1)){
				# if DEBUG == 2
				// print apparentlly tid_dest is dead
				uart_printf(CONSOLE, "Reply %u is dead\r\n", tid_dest);
				# endif
				//-1	tid is not the task id of an existing task.
				PROCS[p].registervalues[0] = -1;
			}
			else if(PROCS[tid_dest - 1].waiting_reply == 0){
				# if DEBUG == 2
				// print apparentlly tid_dest is dead
				uart_printf(CONSOLE, "Reply %u is dead\r\n", tid_dest);
				# endif
				//-2	tid is not the task id of a reply-blocked task.
				PROCS[p].registervalues[0] = -2;
			}
			else 
				reply_helper();
			break;
		case 8: // unblock
			scrSchedule(PID, PROCS[p].priority, READY);
			int tid_unblock = PROCS[p].registervalues[0];
			PROCS[tid_unblock - 1].waiting_reply = 0;
			PROCS[tid_unblock - 1].waiting_send = 1;
			queue_unblock(tid_unblock, PROCS[tid_unblock - 1].priority, READY);
			break;
		default:
			#if DEBUG == 1
			uart_printf(CONSOLE, "Unknown SVC Call\n\r"); // DEBUG PRINT
			#endif
			break;
		}
	}

	Schedule();
	/*
	Begin(&PROCS[p].registervalues[0], PROCS[p].pcpointer, PROCS[p].stackpointer, PROCS[p].pstate); // found in asm.h
	*/
	#if DEBUG >= 1
	uart_printf(CONSOLE, "All Tasks Complete, Press Any Key to Exit\n\r"); // Nothing left // Upon maybe K2, the Kernel may be waiting at this point for user input, or other stuff for Processes to wake up. At this point, the Kernel should in theory spin
	uart_getc(1);
	#endif
	
	EXIT();
}

void Schedule()
{
	PID = scrPick();
	if (PID == -1) return 0;
	
	int p = PID - 1;
	// We need to reset the EL1 stack pointer as well
	
	#if DEBUG == 1
	// uart_printf(CONSOLE, "Beginning pcpointer: %x stackpointer: %x registervalues: %x registervalues: %x %x %x %x %x %x %x %x\r\n", p, PROCS[p].pcpointer, PROCS[p].stackpointer, PROCS[p].registervalues[0], PROCS[p].registervalues[24], PROCS[p].registervalues[25], PROCS[p].registervalues[26], PROCS[p].registervalues[27], PROCS[p].registervalues[28], PROCS[p].registervalues[29], PROCS[p].registervalues[30]); // DEBUG
	// print all the register values
	// uart_printf(CONSOLE, "PID is %u ", PID); // DEBUG PRINT
	// uart_printf(CONSOLE, "PC is %x\n\r", PROCS[p].pcpointer); // DEBUG PRINT
	// uart_printf(CONSOLE, "SP is %x\n\r", PROCS[p].stackpointer); // DEBUG PRINT

	
	// uart_getc(1); /// Spins to stop it from keep on running // DEBUG
	#endif
	
	Begin(&PROCS[p].registervalues[0], PROCS[p].pcpointer, PROCS[p].stackpointer, PROCS[p].pstate); // found in asm.h
	return 0;
}

int KernelCreate(int priority, void (*function)(), int parent)
{	
	// Error Check to see if the pid is correct or not?
	// if (priority < 0) {return -1;} // All prios are valid now
	for (int p = 0; p < NUMPROCS; p++) {
		// uart_printf(CONSOLE, "%u %u\r\n", PRIORITY[p], p); // DEBUG code
		if (PROCS[p].pcpointer == NULL) {
			// This PID is currently not taken
			PROCS[p].pcpointer = function;
			PROCS[p].stackpointer = ((uint8_t*)STACKSTART) + (0x10000 * (p + 1)); // We need to check this
			// Maybe initialize PSTATE???
			// Registers initialized all to 0??
			PROCS[p].parentpid = parent; // MAYBE CHANGE THIS
			PROCS[p].priority = priority;
			PROCS[p].pid = p + 1;
			PROCS[p].pstate = 0;
			PROCS[p].waiting_reply = 0;
			PROCS[p].waiting_send = 0;
			PROCS[p].waiting_recieve_head = 0;
			PROCS[p].waiting_recieve_tail = 0;
			#if DEBUG == 1
			uart_printf(CONSOLE, "Process registervalues: %x, PCPointer: %x, stackpointer: %x parentpid: %x PID: %x created\r\n", &PROCS[p].registervalues[0], PROCS[p].pcpointer, PROCS[p].stackpointer, PROCS[p].parentpid, PROCS[p].pid); // DEBUG
			#endif
			
			scrSchedule(p + 1, PROCS[p].priority, READY);
			
			return p + 1;
		}
	}
	
	return -2;
}
void Kill(int p) // p is the position of the process in the PROCS array
{
	PROCS[p].stackpointer = NULL;
	PROCS[p].pcpointer = NULL;
}
// k2 send receive reply
// k2 send
// Version 1 implementation would have the task directlly tell the kernel
// to put a specific message in the messageDS of the targeted task. 
int Send(int tid, const char *msg, int msglen, char *reply, int replylen){
	asm("svc 5");
	return;
}

int Receive(int *tid, char *msg, int msglen){
	asm("svc 6");
	return;
}
int Reply( int tid, void *reply, int replylen ){
	asm("svc 7");
	return;
}
int UnblockTask(int tid){
	asm("svc 8");
	return;
}
// helper functions 
// mailbox is meant to get the messageDS array of the process. 
// It would work exactlly like a mailbox.

// The Following are EL0 commands
int MyTid()
{
	asm("svc 3");
	return;
}
int MyParentTid()
{
	asm("svc 4");
	return;
}

int Create(int priority, void (*function)()) { // Returns to the Kernel, then calls KernelCreate
	asm("svc 2"); // The Kernel needs to put the pid in x0
	return;
}

// Why is exit SCV 0 
// The difference between an exit and a Yield is Exit do not return back to the priority READY_QUEUE where Yield returns the program back into the priority READY_QUEUE to be ran again. 
void Exit()
{
	asm("svc 0");
	return;
}
void Yield()
{
	asm("svc 1");
	return;
}
