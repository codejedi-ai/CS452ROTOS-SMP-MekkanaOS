#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "k2rps.h"
#include "k3tests.h"
#include "clockserver.h"
#define TRIGGERED 0
#define RELEASED 1
struct free_task_list{
    uint32_t data[NUMPROCS];
    uint32_t tail;
    uint32_t size;
};
void sensor_server_monitor();
void sensor_server();
