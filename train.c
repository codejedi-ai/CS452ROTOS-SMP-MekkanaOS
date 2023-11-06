#include "train.h"
#include "switchworker.h"
#include "sensorserver.h"
/*
typedef enum {
  NODE_NONE,
  NODE_SENSOR,
  NODE_BRANCH,
  NODE_MERGE,
  NODE_ENTER,
  NODE_EXIT,
} node_type;
*/
// inplace return the next node to be hit
struct track_node* next_sensor_node(int sw_server_tid, struct track_node *current_node)
{
	while(current_node->type != NODE_SENSOR && current_node->type != NODE_EXIT){
		// if the current position is a switch then we need to consult the switch table. Which is managed by the switch worker
		if(current_node->type == NODE_BRANCH){
			char sw_state = get_switch(sw_server_tid, current_node->num);
			// this is a switch
			// check the switch table
			// if the switch is set to straight then we need to return the straight node
			if(sw_state == 'S'){
				current_node = current_node->edge[0].dest;
			}
			// if the switch is set to curved then we need to return the curved node
			if(sw_state == 'C'){
				current_node = current_node->edge[1].dest;
			}
		}
		current_node = current_node->edge[0].dest;
	}
	return current_node;
}
// next branch node
struct track_node* next_branch_node(int sw_server_tid, struct track_node *current_node){
	if(current_node->type == NODE_BRANCH)
		return current_node;
	if(current_node->type == NODE_EXIT)
		return current_node;
	return next_branch_node(sw_server_tid, current_node->edge[0].dest);
}
void train_monitor(){
	int sw_server_tid = WhoIs("switch_worker");
	int sensor_server_tid = WhoIs("sensor_server");
	struct train *train_list[TRAIN_MAX];
	int prev_time = get_timerLO();
	int prev_sensor = 0;
	while(1){
		int get_sensor = getNextSensor(sensor_server_tid);
		char *sensor_arr = &get_sensor;
		char *prev_sensor_arr = &prev_sensor;
		char s88_name = sensor_arr[0];
		int sensor_num = sensor_arr[1];

		uart_putc(CONSOLE, prev_sensor_arr[0]);
		uart_printf(CONSOLE, "%d --->", prev_sensor_arr[1]);
		// printed the sensor that is triggered
		uart_putc(CONSOLE, s88_name);
		
		int current_time = get_timerLO();
		int time_diff = current_time - prev_time;
		uart_printf(CONSOLE, "%d Time: %d\r\n", sensor_num, time_diff);
		prev_time = current_time;
		prev_sensor = get_sensor;
		// update the train list
	}
	Exit();
}