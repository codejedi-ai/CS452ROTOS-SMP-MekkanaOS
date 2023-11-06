#include "track_data_new.h"
#define SWITCH_COUNT 18
#define SWITCH_MAX_count 255
void switchSensorTrain_Server();
// void set_switch(int sw_server_tid, uint8_t sw_ind, char state);
// char get_switch(int sw_server_tid, uint8_t sw_ind);

#define TRAIN_MAX 80
// a train can control the sensor direction
struct train
{
    /* data */
    track_node *position;
    uint32_t sensor_time;
    uint32_t dist;
    int speed;
    int train_num;

    uint8_t light;
    
};
struct track_node* next_sensor_node( char* get_switch, struct track_node *current_node, int* dist);
// next branch node
struct track_node* next_branch_node(int sw_server_tid, struct track_node *current_node, int* dist);
void  get_track_node_map(struct track_node *track, struct track_node *trackmap[20][20]);
struct track_node* get_track_node(struct track_node *trackmap[20][20], int s88_id, int sensor_no);