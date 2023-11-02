#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "asm.h"
#include "rpi.h"
#include "util.h"
#include "custstr.h"
# include "systimer.h"
#include "gic.h"
#define DEBUG 3
#define DEBUG_EXIT 1
# define READY 0
# define BLOCKED 1
// it is time to turn READY_QUEUE into a heap
// enqueing on a heap is O(log(n))
// first you 
uint32_t kernelStartTime = 0;
uint8_t NO_PARAMS = 0;
// scrSchedule(pid, priority)
// HEAP IMPLEMENTATION

uint8_t compare_state(struct state a, struct state b)
{
	if (a.priority < b.priority)
		return 1;
	else if (a.priority > b.priority)
		return 2;
	else
	{
		if (a.time < b.time)
			return 1;
		else if (a.time > b.time)
			return 2;
		else
			return 0;
	}
}
void _bubbleUp_state_heap( struct MinHeapState *h, int i)
{
	int parent = (i - 1) / 2;
	if (1 == compare_state(h->harr[i], h->harr[parent]))
	{
		struct state temp = h->harr[parent];
		h->harr[parent] = h->harr[i];
		h->harr[i] = temp;
		_bubbleUp_state_heap(h, parent);
	}
}
void bubbleDown_state_heap( struct MinHeapState *h, int i)
{
	int left = 2 * i + 1;
	int right = 2 * i + 2;
	int smallest = i;
	// if there is no child
	if (left >= h->size && right >= h->size)
		return;
	// if there is only left child
	if (left < h->size && right >= h->size && compare_state(h->harr[left], h->harr[smallest]) == 1)
		smallest = left;
	// no need to look for the case of only right child as it is a complete binary tree, no right child implies no left child implies no child
	// this is given there are two children
	if (left < h->size &&  compare_state(h->harr[left], h->harr[smallest]) == 1)
		smallest = left;
	if (right < h->size &&  compare_state(h->harr[right], h->harr[smallest]) == 1)
		smallest = right;
	if (smallest != i)
	{
		struct state temp = h->harr[smallest];
		h->harr[smallest] = h->harr[i];
		h->harr[i] = temp;
		bubbleDown_state_heap(h, smallest);
	}
}
// add to the heap
void insertKey_state_heap( struct MinHeapState *h, struct state k)
{
	if (h->size == h->capacity)
	{
		return;
	}
	h->harr[h->size] = k;
	_bubbleUp_state_heap(h, h->size);
	h->size++;
}
// check if the heap is empty
uint8_t isEmpty_state_heap( struct MinHeapState *h)
{
	return h->size == 0;
}
// pop the minimum element
struct state extractMin_state_heap( struct MinHeapState *h)
{
	if (h->size <= 0)
		return (struct state){-1,-1};
	if (h->size == 1)
	{
		h->size--;
		return h->harr[0];
	}
	struct state root = h->harr[0];
	h->harr[0] = h->harr[h->size - 1];
	h->size--;
	bubbleDown_state_heap(h, 0);
	return root;
}

void scrSchedule(int pid, uint64_t priority)
{
	struct state currItem = {pid, priority, get_timerLO()};
	struct state nextItem;
	int insert = 0;
	for (int i = 0; i < NUMPROCS; i++) {
		if (READY_QUEUE[i].pid == 0) {
			READY_QUEUE[i] = currItem;
			return 0;	
		}
		else if (READY_QUEUE[i].priority > priority) {
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
void queue_unblock_state(struct state currItem)
{
	// uart_printf(CONSOLE, "queue_unblock: pid = %u priority = %u ready =%u\r\n", pid, priority, ready);
	uint32_t pid = currItem.pid;
	uint32_t priority = currItem.priority;
	// uart_printf(CONSOLE, "queue_unblock: pid = %u priority = %u ready =%u\r\n", pid, priority, ready);
	
	struct state nextItem;
	int insert = 0;
	scrSchedule(pid, priority);
	return 0;
}
// scrSchedule(pid, priority, ready)
int queue_unblock(int pid, uint64_t priority, int ready)
{
	
	// uart_printf(CONSOLE, "queue_unblock: pid = %u priority = %u ready =%u\r\n", pid, priority, ready);
	struct state currItem = {pid, priority, ready};
	queue_unblock_state(currItem);
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
		else if (READY_QUEUE[i].pid > 0) {
			pid = READY_QUEUE[i].pid;
			bump = 1;
		}
		
	}
	if (bump) {READY_QUEUE[NUMPROCS - 1] = emptyItem;}
	return pid;
}

void debugPrint(char *str){
	#if DEBUG == 4
	uart_printf(CONSOLE, str);
	#endif
}
// init the initial variables the system goes by
void InitSys(void* reg)
{	// For some reason, normal init to 0 just.. doesn't work?
	kernelStartTime = get_timerLO();
	STACKSTART = reg;
	READY_HEAP.harr = READY_QUEUE;
	READY_HEAP.capacity = NUMPROCS;
	READY_HEAP.size = 0;
	PID = 0;
	for (int event = 0; event < MAXEVENT; event++){
		AWAIT_INTERRUPT[event].len = 0;
		AWAIT_INTERRUPT[event].eventq_len = 0;
		AWAIT_INTERRUPT[event].eventq_head = 0;
		AWAIT_INTERRUPT[event].eventq_tail = 0;
		for (int jdx = 0; jdx < NUMPROCS; jdx++) {
			AWAIT_INTERRUPT[event].pid_ls[jdx].pid = 0;
			AWAIT_INTERRUPT[event].pid_ls[jdx].priority = 0;
			AWAIT_INTERRUPT[event].pid_ls[jdx].time = 0;
		}
	}
	for (int idx = 1; idx < NUMPROCS; idx++) {
		PROCS[idx].stackpointer = NULL;
		PROCS[idx].pcpointer = NULL;
		PROCS[idx].pstate = 0;
		PROCS[idx].parentpid = 0;
		PROCS[idx].priority = 0;
		// PROCS[idx].queuesize = 0;
		for (int jdx = 0; jdx < 31; jdx++) {
			PROCS[idx].registervalues[jdx] = 10 + jdx;
		}
		
		READY_QUEUE[idx].pid = 0;
		READY_QUEUE[idx].priority = 0;
		READY_QUEUE[idx].time = -1;
		
		
	}
	return 0;
}

void updateRunTimer(){
	uint32_t curtime = get_timerLO();
	PROCS[PID].totaltime += curtime - PROCS[PID].waketime;
}
// ============================ Handel Async

void HandleASYNC(void* sp) // A helper function to pull some c variables into assembly
{
	// We just arrived here, there is stuff on the stack that I do not want to deal with
	updateRunTimer();
	
	uint64_t ASYNC = Save(sp, &PROCS[PID].registervalues[0], &PROCS[PID].pcpointer, &PROCS[PID].stackpointer, &PROCS[PID].pstate);
	ExceptionASYNC(ASYNC);
	Schedule();
	# if DEBUG == 3
	uart_printf(CONSOLE, "ASYNCHandle: ESR is %x\n\r", ASYNC); // DEBUG PRINT
	for (int i = 0; i < NUMPROCS; i++) {
		uart_printf(CONSOLE, "Handle: PID = %u, priority = %u, time = %u, BLOCKED_LIST_PID = %u\r\n", 
					READY_QUEUE[i].pid, 
					READY_QUEUE[i].priority, 
					READY_QUEUE[i].time,
					BLOCKED_LIST[i].pid);
	}
	uart_getc(CONSOLE);
	# endif

	EXIT();
}

int unblock_return(uint32_t interruptid, uint64_t ret){
	# if DEBUG == 4
		uart_printf(CONSOLE, "KERNEL: unblock_return: interruptid = %u, ret = %u, len = %u\r\n", interruptid, ret, AWAIT_INTERRUPT[interruptid].len);
	# endif
	// AWAIT_INTERRUPT[eventType][AWAIT_INTERRUPT_LIST_LEN[eventType]] = currItem;	
	for (int i = 0; i < AWAIT_INTERRUPT[interruptid].len; i++) {
		struct state freed_state = AWAIT_INTERRUPT[interruptid].pid_ls[i];
		# if DEBUG == 4
			uart_printf(CONSOLE, "KERNEL: unblocked-process interruptid = %u, i = %u, pid = %u, priority = %u\r\n", 
						interruptid, i, 
						freed_state.pid, 
						freed_state.priority);
		# endif
		queue_unblock_state(freed_state);
		PROCS[freed_state.pid].registervalues[0] = ret; // the clock was interrupted
	}
	ret = AWAIT_INTERRUPT[interruptid].len;
	AWAIT_INTERRUPT[interruptid].len = 0;
	return ret;
}

void ExceptionASYNC(uint64_t esr_el1){
    
	// make switch case for the exception
	// uart_printf(CONSOLE, "ESR is %x\n\r", esr_el1); // DEBUG PRINT
    uint32_t interruptid = readInterruptId();
    #if DEBUG == 3
    // uart_printf(CONSOLE, "HandleASYNC: ESR is %x\n\r", esr_el1); // DEBUG PRINT
    // uart_printf(CONSOLE, "Asynchronouse SVC Call %x\n\r", interruptid); // DEBUG PRINT
    // uart_printf(CONSOLE, "ESR is %x\n\r", esr_el1); // DEBUG PRINT
    // uart_printf(CONSOLE, "PID = %u\n\r", PID); // DEBUG PRINT
    #endif
	setActiveInterrupt(interruptid);
	// make switch signal
	scrSchedule(PID, PROCS[PID].priority);
	
	// if (CLOCKINTID != interruptid) uart_printf(CONSOLE, "NON CLOCK INTURRUPT\n\r");
	switch (interruptid) {
		case CLOCKINTID:
			// end the interrupt d
			// set the next timer
			// get the time
			set_timerC3(get_timerLO() + 10000);
			resetCS(3);
			unblock_return(CLOCKINTID, 1);
			break;
		case UARTINTER:
			char return_val[8];
			// the first byte of the char is the type of interrupt given
			// the second byte of the char is the line number
			// the last byte of the char is the return value
			uint32_t* RIS_CONSOLE = get_RIS(CONSOLE);
			uint32_t* ICR_CONSOLE = get_ICR(CONSOLE);

			uint32_t* RIS_MARKLIN = get_RIS(MARKLIN);
			uint32_t* ICR_MARKLIN = get_ICR(MARKLIN);
			if((*RIS_CONSOLE) & (0x01 << CTSMIM)){
				// RXIC on the marklin
				return_val[0] = CTSMIM;
				return_val[1] = MARKLIN;
				if (get_CTS(MARKLIN) == 1) return_val[2] = 1; else return_val[2] = 0;
				*ICR_CONSOLE = (0x01 << CTSMIM);
			}else if((*RIS_MARKLIN) & (0x01 << TXIC)){
				// uart_printf(CONSOLE, "TXIC Interrupt ON MARKLIN\n\r");
				// TXIC on the marklin
				return_val[0] = TXIC;
				return_val[1] = MARKLIN;
				return_val[2] = -1;
				# if DEBUG == 4
					// print in green
				uart_printf(CONSOLE, "\033[32m");
				uart_printf(CONSOLE, "KERNEL: TXIC Interrupt ON MARKLIN  0x%x\r\n", *(uint64_t*)return_val);
				// print in white
				uart_printf(CONSOLE, "\033[37m");
				# endif
				*ICR_MARKLIN = (0x01 << TXIC);
			} else if((*RIS_MARKLIN) & (0x01 << RXIC)){
				// RXIC on the marklin
				return_val[0] = RXIC;
				return_val[1] = MARKLIN;
				return_val[2] = uart_getc_modified(MARKLIN);
				# if DEBUG == 4 
				// print in green
				uart_printf(CONSOLE, "\033[32m");
				uart_printf(CONSOLE, "KERNEL: RXIC Interrupt ON MARKLIN  0x%x\r\n", *(uint64_t*)return_val);
				// print in white
				uart_printf(CONSOLE, "\033[37m");
				# endif
				*ICR_MARKLIN = (0x01 << RXIC);
			} else if((*RIS_MARKLIN) & (0x01 << CTSMIM)){
				# if DEBUG == 4 
				// print in green 
				uart_printf(CONSOLE, "\033[32m");
				uart_printf(CONSOLE, "CTSMIM Interrupt ON MARKLIN get_CTS(%u) = %u\n\r", MARKLIN, get_CTS(MARKLIN));
				// print in white
				uart_printf(CONSOLE, "\033[37m");
				# endif
				// RXIC on the marklin
				return_val[0] = CTSMIM;
				return_val[1] = MARKLIN;
				return_val[2] = get_CTS(MARKLIN);
				
				*ICR_MARKLIN = (0x01 << CTSMIM);
			}
			
			if (!unblock_return(interruptid, *(uint64_t*)return_val)){
				# if DEBUG == 4
					// print in red
					uart_printf(CONSOLE, "\033[31m");
					uart_printf(CONSOLE, "UART Interrupt: No one is waiting for this interrupt\n\r");
					// print in white
					uart_printf(CONSOLE, "\033[37m");
				# endif
				AWAIT_INTERRUPT[interruptid].event_q[AWAIT_INTERRUPT[interruptid].eventq_tail] = *(uint64_t*)return_val;
				AWAIT_INTERRUPT[interruptid].eventq_tail++;
				AWAIT_INTERRUPT[interruptid].eventq_tail %= NUMPROCS;
				AWAIT_INTERRUPT[interruptid].eventq_len++;
			}
			
			// INTERRUPT_CLEAR_ACTIVE_REGS(UARTINTER);
			break;
		default:
			# if DEBUG == 4
				uart_printf(CONSOLE, "Unknown Interrupt\n\r");
			# endif
			break;
	}
	INTERRUPT_CLEAR_ACTIVE_REGS(interruptid);
	clear_GICC_EOIR(interruptid);

	// uart_printf(CONSOLE, "Exiting...\r\n");
    
}


// This is the function that is called when a syscall is made
// This is the contextswitch
//==================
void Handle(void* sp) // A helper function to pull some c variables into assembly
{
	// We just arrived here, there is stuff on the stack that I do not want to deal with
	updateRunTimer();
	
	uint64_t esr_el1 = Save(sp, &PROCS[PID].registervalues[0], &PROCS[PID].pcpointer, &PROCS[PID].stackpointer, &PROCS[PID].pstate);
	// this is when the process is officially inturrupted.
	handlerExceptionHelper(esr_el1);
	Schedule();
	# if DEBUG == 3
	uart_printf(CONSOLE, "Handle: ESR is %x\n\r", esr_el1); // DEBUG PRINT
	for (int i = 0; i < NUMPROCS; i++) {
		uart_printf(CONSOLE, "Handle: PID = %u, priority = %u, time = %u, BLOCKED_LIST_PID = %u\r\n", 
					READY_QUEUE[i].pid, 
					READY_QUEUE[i].priority, 
					READY_QUEUE[i].time,
					BLOCKED_LIST[i].pid);
	}
	uart_getc(CONSOLE);
	# endif

	Exit();
}
int8_t dead(int8_t p){
	return (PROCS[p].stackpointer == NULL && PROCS[p].pcpointer == NULL);
}
// Each parameter is now stored in the registers
void send_helper(int tid){
    // tid is the TID of the target task
	# if DEBUG == 2
	// print the function called
	// uart_printf(CONSOLE, "===============\r\n Send Helper Called:\r\n");
	# endif
	// Debug
	char *msg = PROCS[PID].registervalues[1];
	uint64_t msglen = PROCS[PID].registervalues[2];
	char *reply = PROCS[PID].registervalues[3];
	uint64_t replylen = PROCS[PID].registervalues[4];

	// This puts the message into the messageDS of the target task
	PROCS[PID].message_sent.tid = tid;
	PROCS[PID].message_sent.msg = msg;
	PROCS[PID].message_sent.msglen = msglen;
	PROCS[PID].message_sent.reply = reply;
	PROCS[PID].message_sent.replylen = replylen;
	PROCS[PID].waiting_reply = 1;
	// This is the target task, if it is waiting_send then we need ot remove the waiting send and unblock the task
	int tail = PROCS[tid].waiting_recieve_tail;
	PROCS[tid].message_recieved[tail].tid = PID; // This is the tricky part for the recieved it should be the sender's pid
	PROCS[tid].message_recieved[tail].msg = msg;
	PROCS[tid].message_recieved[tail].msglen = msglen;
	PROCS[tid].message_recieved[tail].reply = reply;
	PROCS[tid].message_recieved[tail].replylen = replylen;
	// // uart_printf(CONSOLE, "reply addr is %x\r\n", reply);
	PROCS[tid].waiting_recieve_tail++;
	PROCS[tid].waiting_recieve_tail %= QUEUESIZE;
	PROCS[tid].queuesize++;
	if (PROCS[tid].waiting_send == 1){
		recieve_helper(tid);
	}
	// At this point we need to wake up the message processing task
	
}
// It assumes that the messageDS is not empty
// recieve takes a message from the mailbox and returns the message inplace
void recieve_helper(int p){
	int head = PROCS[p].waiting_recieve_head;
	int tail = PROCS[p].waiting_recieve_tail;
	int *tid =  PROCS[p].registervalues[0]; // This is a memory address for the TID
	char *msg = PROCS[p].registervalues[1]; // this is another memory address for the message
	int msglen = PROCS[p].registervalues[2];
	if (head == tail){
		PROCS[p].waiting_send = 1;
		return;
	}
	PROCS[p].waiting_send = 0;
	uint64_t curread_message_length = PROCS[p].message_recieved[head].msglen;
	char *curread_msg = PROCS[p].message_recieved[head].msg;
	int sender_tid = PROCS[p].message_recieved[head].tid;
	*tid = sender_tid;
    msglen = min(msglen, curread_message_length);
	memcpy(msg, curread_msg, msglen);
    PROCS[p].waiting_recieve_head++;
	if(PROCS[p].queuesize > 0){
		PROCS[p].queuesize--;
	}
	PROCS[p].waiting_recieve_head %= QUEUESIZE;
	PROCS[p].registervalues[0] = msglen;
	queue_unblock(p, PROCS[p].priority);
	// return msglen;
	
	// this is the sender process. The sender is ready for a reply
}
void reply_helper(uint64_t p_replied){
	# if DEBUG == 2
	// uart_printf(CONSOLE, "===============\r\n Reply Helper Called:\r\n");
	# endif
	// replies to the PID;
	char *reply = (char *)PROCS[PID].registervalues[1];
	uint64_t replylen = PROCS[PID].registervalues[2];
	char *reply_buffer = PROCS[p_replied].message_sent.reply;
	uint64_t reply_buffer_len = PROCS[p_replied].message_sent.replylen;
    
    replylen = min(reply_buffer_len, replylen);
	memcpy(reply_buffer, reply, replylen);
	queue_unblock(p_replied, PROCS[p_replied].priority);
	// return a reply length for the send function
	PROCS[p_replied].registervalues[0] = replylen;
	PROCS[p_replied].waiting_reply = 0;
	// return for the reply function
	PROCS[PID].registervalues[0] = replylen;
	
}

void handlerExceptionHelper(uint64_t esr_el1)
{
	#if DEBUG == 1
	// uart_printf(CONSOLE, "ESR is %x\n\r", esr_el1); // DEBUG PRINT
	#endif
	// PID is the currentlly running process
	if (esr_el1 >> 24 == 86) { // an svc call has occured!
		int i = esr_el1 % 0x1000000;
		switch (i) {
		
		case 0: // Exit
			Kill(PID);
			break;
		case 1: // Yield
			scrSchedule(PID, PROCS[PID].priority);
			break;
		case 2: // Create
			scrSchedule(PID, PROCS[PID].priority);
			int ret = KernelCreate(PROCS[PID].registervalues[0], PROCS[PID].registervalues[1], PID);
			PROCS[PID].registervalues[0] = ret;
			break;
		case 3: // mytid
			scrSchedule(PID, PROCS[PID].priority);
			PROCS[PID].registervalues[0] = PROCS[PID].pid;
			break;
		case 4: // parenttid
			// // uart_printf(CONSOLE, "PPID is %u\n\r", PROCS[PID].parentpid); // DEBUG PRINT
		
			scrSchedule(PID, PROCS[PID].priority);
			PROCS[PID].registervalues[0] = PROCS[PID].parentpid;
			break;
		case 5: // send blocks and unblocks other tasks
			// This unblocks the recieving task
			int dest_p = PROCS[PID].registervalues[0];
			if (dead(dest_p)){
				scrSchedule(PID, PROCS[PID].priority);
				PROCS[PID].registervalues[0] = -1;
			} else if (PROCS[dest_p].queuesize >= QUEUESIZE){
				scrSchedule(PID, PROCS[PID].priority);
				PROCS[PID].registervalues[0] = -2;
			}
			else
			{
				BLOCKED_LIST[PID].pid = PID;
				BLOCKED_LIST[PID].priority = PROCS[PID].priority;
				BLOCKED_LIST[PID].time = get_timerLO();
				send_helper(dest_p);
			}
			// however there is another case in which the task unblocks
			break;
		case 6: // recieve blocks

			// There is a message in the mailbox
			BLOCKED_LIST[PID].pid = PID;
			BLOCKED_LIST[PID].priority = PROCS[PID].priority;
			BLOCKED_LIST[PID].time = get_timerLO();
			recieve_helper(PID);
			
			break;
		case 7: // reply
			scrSchedule(PID, PROCS[PID].priority);
			uint64_t tid_dest = PROCS[PID].registervalues[0];
			if(dead(tid_dest)){
				# if DEBUG == 2
				// print apparentlly tid_dest is dead
				// uart_printf(CONSOLE, "Reply %u is dead\r\n", tid_dest);
				# endif
				//-1	tid is not the task id of an existing task.
				PROCS[PID].registervalues[0] = -1;
			}
			else if(PROCS[tid_dest].waiting_reply != 1){
				// the message is not recieved, thus reply is not possible
				// messsage is in three statges
				// sent, recieved, reply
				// not-sent 0 returns -2
				// sent 1 returns -3
				// recieved 2
				# if DEBUG == 2
				// print apparentlly tid_dest is dead
				// uart_printf(CONSOLE, "Reply %u is dead\r\n", tid_dest);
				# endif
				//-2	tid is not the task id of a reply-blocked task.
				PROCS[PID].registervalues[0] = -2 - PROCS[tid_dest].waiting_reply;
			}
			else 
				reply_helper(tid_dest);
			break;
		case 8: // MyPriority
			scrSchedule(PID, PROCS[PID].priority);
			PROCS[PID].registervalues[0] = PROCS[PID].priority;
			break;
		case 10: // get the interrupt
			uint64_t eventType = PROCS[PID].registervalues[0];
			PROCS[PID].registervalues[0] = -1;
			if (checkActiveInterrupt(eventType)){
				// check the interrupt queue, if the queue is empty then block the task
				if (AWAIT_INTERRUPT[eventType].eventq_len){
					// pop the queue
					scrSchedule(PID, PROCS[PID].priority);
					PROCS[PID].registervalues[0] = AWAIT_INTERRUPT[eventType].event_q[AWAIT_INTERRUPT[eventType].eventq_head];
					AWAIT_INTERRUPT[eventType].eventq_head++;
					AWAIT_INTERRUPT[eventType].eventq_head %= NUMPROCS;
					AWAIT_INTERRUPT[eventType].eventq_len--;

				}
				else{
					BLOCKED_LIST[PID].pid = PID;
					BLOCKED_LIST[PID].priority = PROCS[PID].priority;
					BLOCKED_LIST[PID].time = get_timerLO();
					struct state currItem = {PID, PROCS[PID].priority};
					AWAIT_INTERRUPT[eventType].pid_ls[AWAIT_INTERRUPT[eventType].len] = currItem;
					AWAIT_INTERRUPT[eventType].len = AWAIT_INTERRUPT[eventType].len++;
				}
			}else{
				scrSchedule(PID, PROCS[PID].priority);
			}

			break;
		case 11: // get total time
			// uart_printf(CONSOLE, "Awaiting Interrupt %u\r\n", eventType);
			scrSchedule(PID, PROCS[PID].priority);
			PROCS[PID].registervalues[0] = PROCS[PID].totaltime;
			break;
		case 12: // get kernel runtime
			scrSchedule(PID, PROCS[PID].priority);
			PROCS[PID].registervalues[0] = get_timerLO() - kernelStartTime;
			break;

		default:
			scrSchedule(PID, PROCS[PID].priority);
			# if DEBUG == 3
			uart_printf(CONSOLE, "Unknown SVC Call: %x\n\r", i); // DEBUG PRINT
			# endif
			break;
		}
	}
}

void Schedule()
{
	PID = scrPick();
	if (PID == -1) return 0;
	PROCS[PID].waketime = get_timerLO();
	Begin(&PROCS[PID].registervalues[0], PROCS[PID].pcpointer, PROCS[PID].stackpointer, PROCS[PID].pstate); // found in asm.h
	return 0;
}

int KernelCreate(uint64_t priority, void (*function)(), int parent)
{	
	// Error Check to see if the pid is correct or not?
	// if (priority < 0) {return -1;} // All prios are valid now
	for (int p = 1; p <= NUMPROCS; p++) {
        if (PROCS[p].pcpointer == NULL) {
			// This PID is currently not taken
			PROCS[p].pcpointer = function;
			PROCS[p].stackpointer = ((uint8_t*)STACKSTART) + (0x10000 * (p)); // We need to check this
			// Maybe initialize PSTATE???
			// Registers initialized all to 0??
			PROCS[p].parentpid = parent; // MAYBE CHANGE THIS
			PROCS[p].priority = priority;
			PROCS[p].pid = p;
			PROCS[p].pstate = 0;
			PROCS[p].waiting_reply = 0;
			PROCS[p].waiting_send = 0;
			PROCS[p].waiting_recieve_head = 0;
			PROCS[p].waiting_recieve_tail = 0;
			PROCS[p].queuesize = 0;
			PROCS[p].totaltime = 0;
			scrSchedule(p, PROCS[p].priority);
			
			return p;
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
int MyPriority(int tid){
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

int Create(uint64_t priority, void (*function)()) { // Returns to the Kernel, then calls KernelCreate
	asm("svc 2"); // The Kernel needs to put the pid in x0
	return;
}

int CreateArgs(uint64_t priority, void (*function)(), uint64_t argsno, uint64_t *args) { // Returns to the Kernel, then calls KernelCreate
	asm("svc 9"); // The Kernel needs to put the pid in x0
	return;
}
int AwaitEvent(int eventType){ // Returns to the Kernel, then calls KernelCreate
	asm("svc 10"); // The Kernel needs to put the pid in x0
	return;
}
int GetRuntime(){ // Returns to the Kernel, then calls KernelCreate
	asm("svc 11"); // The Kernel needs to put the pid in x0
	return;
}
int GetKernelRuntime(){ // Returns to the Kernel, then calls KernelCreate
	asm("svc 12"); // The Kernel needs to put the pid in x0
	return;
}
// Why is exit SCV 0 
// The difference between an exit and a Yield is Exit do not return back to the priority READY_QUEUE where Yield returns the program back into the priority READY_QUEUE to be ran again. 
void Exit()
{
	Deregister();
	asm("svc 0");
	return;
}
void Yield()
{
	asm("svc 1");
	return;
}