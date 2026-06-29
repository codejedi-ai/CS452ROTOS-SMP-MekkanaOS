#include "syscall.h"
#include "processes.h"
#include "nameserver.h"
#include "asm.h"
#include "rpi.h"
#include "util.h"
#include "custmath.h"
#include "systimer.h"
#include "gic.h"
#include "kernel_state.h"
#include "kernel_api.h"
#include "sched_stubs.h"
#include "ipc_stubs.h"

#ifndef DEBUG
#define DEBUG 5
#endif
#ifndef DEBUG_EXIT
#define DEBUG_EXIT 1
#endif

#define READY 0

extern void EXIT(void);
void setActiveInterrupt(uint32_t interrupt_id);
void INTERRUPT_CLEAR_ACTIVE_REGS(uint32_t interrupt_id);
void handlerExceptionHelper(uint64_t esr_el1);

uint8_t NO_PARAMS = 0;

void debugPrint(char *str)
{
#if DEBUG == 4
	uart_printf(CONSOLE, str);
#else
	(void)str;
#endif
}

void InitSys(void *reg)
{
	sched_init(reg);
}

void HandleASYNC(void *sp)
{
	int p = PID - 1;
	updateRunTimer();

	uint64_t ASYNC = Save(sp, &PROCS[p].registervalues[0], &PROCS[p].pcpointer, &PROCS[p].stackpointer, &PROCS[p].pstate);
	ExceptionASYNC(ASYNC);
	Schedule();

#if DEBUG >= 1
	uart_printf(CONSOLE, "All Tasks Complete, Press Any Key to Exit\n\r");
	uart_getc(1);
#endif
#if DEBUG_EXIT >= 1
	EXIT();
#else
	while (1)
	{
		asm("wfi");
	}
#endif
}

int unblock_return(uint32_t interruptid, uint64_t ret)
{
#if DEBUG == 4
	if (interruptid != CLOCKINTID)
		uart_printf(CONSOLE, "KERNEL: unblock_return: interruptid = %u, ret = %u, len = %u
", interruptid, ret, AWAIT_INTERRUPT[interruptid].len);
#endif
	return sched_unblock_event(interruptid, ret);
}

void ExceptionASYNC(uint64_t esr_el1)
{
	(void)esr_el1;
	int p = PID - 1;

	uint32_t interruptid = readInterruptId();
	if (interruptid >= 1020)
		return;

	setActiveInterrupt(interruptid);
	scrSchedule((int)PID, PROCS[p].priority);

	switch (interruptid)
	{
	case CLOCKINTID:
		set_timerC3(get_timerLO() + 10000);
		resetCS(3);
		unblock_return(CLOCKINTID, 1);
		break;
	case UARTINTER:
	{
		char return_val[8];
		volatile uint32_t *RIS_CONSOLE = get_RIS(CONSOLE);
		volatile uint32_t *ICR_CONSOLE = get_ICR(CONSOLE);
		volatile uint32_t *RIS_MARKLIN = get_RIS(MARKLIN);
		volatile uint32_t *ICR_MARKLIN = get_ICR(MARKLIN);
		if ((*RIS_CONSOLE) & (0x01 << CTSMIM))
		{
			return_val[0] = CTSMIM;
			return_val[1] = MARKLIN;
			if (get_CTS(MARKLIN) == 1)
				return_val[2] = 1;
			else
				return_val[2] = 0;
			*ICR_CONSOLE = (0x01 << CTSMIM);
		}
		else if ((*RIS_CONSOLE) & ((0x01 << RXIC) | (0x01 << 6)))
		{
			return_val[0] = RXIC;
			return_val[1] = CONSOLE;
			return_val[2] = uart_getc_modified(CONSOLE);
			*ICR_CONSOLE = ((0x01 << RXIC) | (0x01 << 6));
			while (uart_getc_queue(CONSOLE))
			{
				char extra[8] = {0};
				extra[0] = RXIC;
				extra[1] = CONSOLE;
				extra[2] = uart_getc_modified(CONSOLE);
				AWAIT_INTERRUPT[interruptid].event_q[AWAIT_INTERRUPT[interruptid].eventq_tail] = *(uint64_t *)extra;
				AWAIT_INTERRUPT[interruptid].eventq_tail++;
				AWAIT_INTERRUPT[interruptid].eventq_tail %= NUMPROCS;
				AWAIT_INTERRUPT[interruptid].eventq_len++;
			}
		}
		else if ((*RIS_MARKLIN) & (0x01 << TXIC))
		{
			return_val[0] = TXIC;
			return_val[1] = MARKLIN;
			return_val[2] = -1;
			*ICR_MARKLIN = (0x01 << TXIC);
		}
		else if ((*RIS_MARKLIN) & (0x01 << RXIC))
		{
			return_val[0] = RXIC;
			return_val[1] = MARKLIN;
			return_val[2] = uart_getc_modified(MARKLIN);
			*ICR_MARKLIN = (0x01 << RXIC);
		}
		else if ((*RIS_MARKLIN) & (0x01 << CTSMIM))
		{
			return_val[0] = CTSMIM;
			return_val[1] = MARKLIN;
			return_val[2] = get_CTS(MARKLIN);
			*ICR_MARKLIN = (0x01 << CTSMIM);
		}

		if (!unblock_return(interruptid, *(uint64_t *)return_val))
		{
			AWAIT_INTERRUPT[interruptid].event_q[AWAIT_INTERRUPT[interruptid].eventq_tail] = *(uint64_t *)return_val;
			AWAIT_INTERRUPT[interruptid].eventq_tail++;
			AWAIT_INTERRUPT[interruptid].eventq_tail %= NUMPROCS;
			AWAIT_INTERRUPT[interruptid].eventq_len++;
		}
		break;
	}
	default:
#if DEBUG == 4
		uart_printf(CONSOLE, "Unknown Interrupt\n\r");
#endif
		break;
	}
	INTERRUPT_CLEAR_ACTIVE_REGS(interruptid);
	clear_GICC_EOIR(interruptid);
}

void Handle(void *sp)
{
	int p = PID - 1;
	updateRunTimer();

	uint64_t esr_el1 = Save(sp, &PROCS[p].registervalues[0], &PROCS[p].pcpointer, &PROCS[p].stackpointer, &PROCS[p].pstate);
	handlerExceptionHelper(esr_el1);
	Schedule();
#if DEBUG_EXIT >= 1
	uart_printf(CONSOLE, "All Tasks Complete, Press Any Key to Exit\n\r");
	uart_getc(1);
	EXIT();
#endif
}

void handlerExceptionHelper(uint64_t esr_el1)
{
	int p = PID - 1;
	if (esr_el1 >> 24 == 86)
	{
		int i = (int)(esr_el1 % 0x1000000);
		PROCS[p].pcpointer = (void (*)(void))((uint64_t)PROCS[p].pcpointer + 4);

		switch (i)
		{
		case 0:
			Kill(p);
			break;
		case 1:
			scrSchedule((int)PID, PROCS[p].priority);
			break;
		case 2:
			scrSchedule((int)PID, PROCS[p].priority);
			PROCS[p].registervalues[0] = (uint64_t)KernelCreate((uint8_t)PROCS[p].registervalues[0], (void (*)(void))PROCS[p].registervalues[1], (int)PID);
			break;
		case 3:
			scrSchedule((int)PID, PROCS[p].priority);
			PROCS[p].registervalues[0] = (uint64_t)PROCS[p].pid;
			break;
		case 4:
			scrSchedule((int)PID, PROCS[p].priority);
			PROCS[p].registervalues[0] = (uint64_t)PROCS[p].parentpid;
			break;
		case 5:
			ipc_handle_send(p);
			break;
		case 6:
			ipc_handle_receive(p);
			break;
		case 7:
			ipc_handle_reply(p);
			break;
		case 8:
			scrSchedule((int)PID, PROCS[p].priority);
			PROCS[p].registervalues[0] = PROCS[p].priority;
			break;
		case 9:
		{
			scrSchedule((int)PID, PROCS[p].priority);
			uint64_t retd = (uint64_t)KernelCreate((uint8_t)PROCS[p].registervalues[0], (void (*)(void))PROCS[p].registervalues[1], (int)PID);
			if (PROCS[p].registervalues[2] > 0)
			{
				for (int j = 0; j < 8; j++)
					PROCS[retd - 1].registervalues[j] = ((int64_t *)PROCS[p].registervalues[3])[j];
				if (PROCS[p].registervalues[2] > 8)
				{
					int64_t *newsp = (int64_t *)PROCS[retd - 1].stackpointer;
					uint8_t stack_offset = (uint8_t)(PROCS[p].registervalues[2] - 8);
					newsp = newsp - (PROCS[p].registervalues[2] - 8);
					if (stack_offset > 0)
					{
						for (int j = 0; j < stack_offset; j++)
							newsp[j] = ((int64_t *)PROCS[p].registervalues[3])[j + 8];
						PROCS[retd - 1].stackpointer = (void *)newsp;
					}
				}
			}
			PROCS[p].registervalues[0] = retd;
			break;
		}
		case 10:
		{
			uint64_t eventType = PROCS[p].registervalues[0];
			PROCS[p].registervalues[0] = (uint64_t)-1;
			if (checkActiveInterrupt(eventType))
			{
				if (AWAIT_INTERRUPT[eventType].eventq_len)
				{
					scrSchedule((int)PID, PROCS[p].priority);
					PROCS[p].registervalues[0] = AWAIT_INTERRUPT[eventType].event_q[AWAIT_INTERRUPT[eventType].eventq_head];
					AWAIT_INTERRUPT[eventType].eventq_head++;
					AWAIT_INTERRUPT[eventType].eventq_head %= NUMPROCS;
					AWAIT_INTERRUPT[eventType].eventq_len--;
				}
				else
				{
					block((int)PID, PROCS[p].priority);
					struct state currItem = {(int)PID, PROCS[p].priority, SCHED_BLOCKED};
					AWAIT_INTERRUPT[eventType].pid_ls[AWAIT_INTERRUPT[eventType].len] = currItem;
					AWAIT_INTERRUPT[eventType].len++;
				}
			}
			else
				scrSchedule((int)PID, PROCS[p].priority);
			break;
		}
		case 11:
			scrSchedule((int)PID, PROCS[p].priority);
			PROCS[p].registervalues[0] = PROCS[p].totaltime;
			break;
		case 12:
			scrSchedule((int)PID, PROCS[p].priority);
			PROCS[p].registervalues[0] = get_timerLO() - kernelStartTime;
			break;
		default:
			scrSchedule((int)PID, PROCS[p].priority);
#if DEBUG == 3
			uart_printf(CONSOLE, "Unknown SVC Call: %x\n\r", i);
#endif
			break;
		}
	}
}

int Send(int tid, const char *msg, int msglen, char *reply, int replylen)
{
	(void)tid;
	(void)msg;
	(void)msglen;
	(void)reply;
	(void)replylen;
	asm("svc 5");
	return 0;
}

int Receive(int *tid, char *msg, int msglen)
{
	(void)tid;
	(void)msg;
	(void)msglen;
	asm("svc 6");
	return 0;
}

int Reply(int tid, void *reply, int replylen)
{
	(void)tid;
	(void)reply;
	(void)replylen;
	asm("svc 7");
	return 0;
}

int MyPriority(void)
{
	asm("svc 8");
	return 0;
}

int MyTid(void)
{
	asm("svc 3");
	return 0;
}

int MyParentTid(void)
{
	asm("svc 4");
	return 0;
}

int Create(uint8_t priority, void (*function)())
{
	(void)priority;
	(void)function;
	asm("svc 2");
	return 0;
}

int CreateArgs(uint8_t priority, void (*function)(), uint64_t argsno, uint64_t *args)
{
	(void)priority;
	(void)function;
	(void)argsno;
	(void)args;
	asm("svc 9");
	return 0;
}

int AwaitEvent(int eventType)
{
	(void)eventType;
	asm("svc 10");
	return 0;
}

int GetRuntime(void)
{
	asm("svc 11");
	return 0;
}

int GetKernelRuntime(void)
{
	asm("svc 12");
	return 0;
}

void Exit(void)
{
	asm("svc 0");
}

void Yield(void)
{
	asm("svc 1");
}
