#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "util.h"
#include "nameserver.h"
#include "custstr.h"
#include "systimer.h"
#include "int64voodoo.h"
void initPlayer(uint64_t N, uint64_t player_type, uint64_t priority);