#include "track_data_new.h"
// a train can control the sensor direction
#define TRAIN_MAX 80
struct train
{
    /* data */
    track_node *position;
    int speed;
    int train_num;
    int light;
};
void train_monitor();