#include "printmethod.h"
#include "nameserver.h"
#include "ioserver.h"
#define SWITCH_COUNT 18
#define SWITCH_MAX_count 255
void switch_worker();
void set_switch(int sw_server_tid, uint8_t sw_ind, char state);
char get_switch(int sw_server_tid, uint8_t sw_ind);