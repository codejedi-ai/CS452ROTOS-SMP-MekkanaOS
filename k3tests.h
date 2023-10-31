#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "gameserver.h"
#include "custstr.h"
#include "systimer.h"
#include "clockserver.h"

int32_t init_clock_proc(uint64_t priority, char *clockname, int delay, int numberOfDelays);
int k3ExecuteCommands(char *command, char **num, int command_part_count);
void k3_clock_proc();