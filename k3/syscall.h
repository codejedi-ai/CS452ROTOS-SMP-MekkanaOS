#ifndef _syscall_h_
#define _syscall_h_ 1
#include "asm.h"
#include <stdint.h>
#define NUMPROCS 20
#define QUEUESIZE 50
static void *STACKSTART;
// This is the PID of the currentlly running process
static uint32_t PID = 0;




// static const int NUMPROCS = 20; // Deprecated

void InitSys(void* reg);
void Handle();
void Exception(uint64_t esr_el1);

void Kill(int p);
int KernelCreate(int priority, void (*function)(), int parent);
int Create(int priority, void (*function)());
int CreateArgs(int priority, void (*function)(), uint64_t argsno, uint64_t *args);
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
    uint64_t msglen;
    char *reply;
    uint64_t replylen;
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
	struct message message_recieved[QUEUESIZE];
	uint8_t waiting_recieve_head;
	uint8_t waiting_recieve_tail;
	uint8_t queuesize;
	uint8_t waiting_recieve;
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
static struct process PROCS[NUMPROCS];
static struct state READY_QUEUE[NUMPROCS];
static struct state BLOCKED_QUEUE[NUMPROCS];
void scrSchedule(int pid, int priority, int ready);
int scrPick();
void HandleASYNC(void* sp);
void ExceptionASYNC(uint64_t esr_el1);
#endif
