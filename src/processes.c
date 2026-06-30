#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"

#include "custstr.h"
#include "nameserver.h"
#include "gameserver.h"
#include "clockserver.h"
#include "systimer.h"
#include "io_api.h"

#define UARTINTER 153

void init_trains(void)
{
	Exit();
}

void init_solonoids(void)
{
	Exit();
}
