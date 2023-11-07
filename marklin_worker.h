//#ifndef _MCC_h_
//#define _MCC_h_

#define TRIGGERED 0
#define RELEASED 1
#define SWITCH_COUNT 18
struct free_task_list{
    uint32_t data[NUMPROCS];
    uint32_t tail;
    uint32_t size;
};

void MCW();
void MCW_read_notifier();
void set_solonoid(int MCW_tid, uint8_t sol_id, char state);
void set_train_speed(int MCW_tid, uint8_t train_ind, char speed);
void set_reverse(int MCW_tid, uint8_t train_id, char speed);
//#endif
