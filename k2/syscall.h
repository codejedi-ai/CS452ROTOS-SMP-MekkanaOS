#ifndef _syscall_h_
#define _syscall_h_ 1
#include "asm.h"
#include <stdint.h>
#define NUMPROCS 20


void InitSys(void* reg);
void Handle();
void Exception(uint64_t esr_el1);

void Kill(int p);
int KernelCreate(int priority, void (*function)(), int parent);
int Create(int priority, void (*function)());
int CreateArgs(int priority, void (*function)(), int8_t argsno, int64_t *args);
void Schedule();

int MyTid();
int MyParentTid();

void Yield();
void Exit(); 

// This is where K2 starts
int Send(int tid, const char *msg, int msglen, char *reply, int replylen);
int Receive(int *tid, char *msg, int msglen);
int Reply( int tid, void *reply, int replylen );
struct message{
    int tid; // to which task
    char *msg;
    int msglen;
    char *reply;
    int replylen;
};

struct process {
	void *stackpointer;
	void (*pcpointer)();
	uint32_t pstate;
	int parentpid;
	int pid;
	int priority;
	uint64_t registervalues[31];
	// define an array of messages such would be held in memory for each process.
	// A kernel call is needed to get the message array for the process.
	struct message message_sent;
	struct message message_recieved[50];
	uint8_t waiting_recieve_head;
	uint8_t waiting_recieve_tail;
	uint8_t waiting_reply;
	uint8_t waiting_send;
};

# define READY 0
# define BLOCKED 1
struct state {
	int pid;
	int priority;
	int ready;
};

void scrSchedule(int pid, int priority, int ready);
int scrPick();

#endif
