#include "track_data_new.h"
#define SWITCH_COUNT 18
#define SWITCH_MAX_count 255

// void set_switch(int track_server_tid, uint8_t sw_ind, char state);
// char get_switch(int track_server_tid, uint8_t sw_ind);

#define TRAIN_MAX 80
// a train can control the sensor direction
struct train
{
    /* data */
    int sensor_hist[6][TRAIN_MAX];
    int sensor_pushed_count;
    track_node *position;
    track_node *predict_sensor;
    uint32_t sensor_time;
    uint32_t dist;
    int speed;
    int train_num;
    int dist_to_next_sensor;
    uint8_t light;
    
};
#define PREDICTNODECOL 130
#define TABLEROW 50
#define TABLECOL 50
#define SENSORROW 50
#define SENSORCOL 0
void print_table_headers();
void get_sensor_pushed(int track_server_tid, int sensor_pushed[], int train_max);
void get_switch_states(int track_server_tid, char sw_states[], int switch_count);
void init_track(int track_server_tid, char track_id);
void init_train(int track_server_tid, char train_no, char s88_id, char sensor_id);
void deregister_train(int track_server_tid, char train_no);
uint64_t awit_sensor(int track_server_tid);

// Processes
void track_server();