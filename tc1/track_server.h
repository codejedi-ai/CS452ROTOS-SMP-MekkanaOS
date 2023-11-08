#include "train.h"
#define SWITCH_COUNT 18
#define SWITCH_MAX_count 255

// void set_switch(int track_server_tid, uint8_t sw_ind, char state);
// char get_switch(int track_server_tid, uint8_t sw_ind);

#define TRAIN_MAX 80
// a train can control the sensor direction

#define PREDICTNODECOL 130
#define TABLEROW 50
#define TABLECOL 50
#define SENSORROW 50
#define SENSORCOL 0
void get_sensor_pushed(int track_server_tid, struct track_node track[TRACK_MAX], int tracks_max);
char get_track_id(int track_server_tid);
void init_track(int track_server_tid, char track_id);
void init_train(int track_server_tid, char train_no, char s88_id, char sensor_id);
void deregister_train(int track_server_tid, char train_no);
uint64_t await_sensor(int track_server_tid);

// Processes
void track_server();