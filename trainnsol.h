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
#define PREDICTNODECOL 130
#define TABLEROW 50
#define TABLECOL 50
#define SENSORROW 50
#define SENSORCOL 0
void print_table_headers();