#include "rpi.h"
#include "util.h"
#include "nameserver.h"
#include "ioserver.h"
#include "trainnsol.h"

struct track_node* next_sensor_node(char* get_switch, struct track_node *current_node, int* dist)
{
  *dist = 0;
  // the current node is a sensor node then the for loop would not run
  struct track_node *start_node = current_node;
	while((start_node == current_node) || (current_node->type != NODE_SENSOR && current_node->type != NODE_EXIT)){
		// if the current position is a switch then we need to consult the switch table. Which is managed by the switch worker
		if(current_node->type == NODE_BRANCH){
			char sw_state = get_switch[current_node->num];
			// this is a switch
			// check the switch table
			// if the switch is set to straight then we need to return the straight node
			if(sw_state == 'S'){
				current_node = current_node->edge[0].dest;
        *dist += current_node->edge[0].dist;
      }
			// if the switch is set to curved then we need to return the curved node
			if(sw_state == 'C'){
				current_node = current_node->edge[1].dest;
        *dist += current_node->edge[1].dist;
			}
		}
		current_node = current_node->edge[0].dest;
    *dist += current_node->edge[0].dist;
  }
	return current_node;
}
// next branch node
struct track_node* next_branch_node(int sw_server_tid, struct track_node *current_node, int* dist){
  /*
	if(current_node->type == NODE_BRANCH)
		return current_node;
	if(current_node->type == NODE_EXIT)
		return current_node;
	return next_branch_node(sw_server_tid, current_node->edge[0].dest);
  */
  // write it iteratively
  *dist = 0;
  struct track_node *start_node = current_node;
	while((start_node == current_node) || (current_node->type != NODE_BRANCH && current_node->type != NODE_EXIT)){
    *dist += current_node->edge[0].dist;
    current_node = current_node->edge[0].dest;
  }
  return current_node;
}
void  get_track_node_map(struct track_node *track, struct track_node *trackmap[20][20]){
	// iterate through all the SENSOR_NODEs and find the one that matches the s88_id and sensor_no
	// the s88_id is the s88 that is triggered in alphabet A,B,C,D
	// the sensor_no is the sensor that is triggered in the s88 from 1 - 16
	// the naming convention is A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, D1, D2, D3, D4.....
	for(int i = 0; i < TRACK_MAX; i++){
		if(track[i].type == NODE_SENSOR){
			// get name
			char *name = track[i].name;
			// get the s88_id the first character
			char s88_id = name[0] - 'A';
			// get the sensor_no the number that is after the first character
			int64_t sensor_no = atoi_64(&name[1]) - 1;
			//uart_printf(CONSOLE, "name:%s s88_id = %d, sensor_no = %d\r\n",name , s88_id, sensor_no);
			trackmap[s88_id][sensor_no] = &track[i];
		}
	}
}
struct track_node* get_track_node(struct track_node *trackmap[20][20], int s88_id, int sensor_no){
	// iterate through all the SENSOR_NODEs and find the one that matches the s88_id and sensor_no
	// the s88_id is the s88 that is triggered in alphabet A,B,C,D
	// the sensor_no is the sensor that is triggered in the s88 from 1 - 16
	// the naming convention is A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4, D1, D2, D3, D4.....
	sensor_no--;
	//uart_printf(CONSOLE, "name:%s s88_id = %d, sensor_no = %d\r\n",trackmap[s88_id][sensor_no]->name, s88_id, sensor_no);
	return trackmap[s88_id][sensor_no];
}
void print_sw_states(char *sw_states, uint32_t r, uint32_t c, uint8_t sw_ind){
  uart_printf(CONSOLE,"\033[%u;%uH",r,c);
  uart_puts(CONSOLE, "SW");
  uint8_t middle_Sw[] ={0x99,0x9a,0x9b,0x9c};
  for (uint32_t i = 0; i < 4; i ++){
    if (sw_ind == middle_Sw[i]){
      uart_printf(CONSOLE,"\033[%u;%uH",r + i + SWITCH_COUNT + 1, c);
      uart_printf(CONSOLE,"T%x: ",  middle_Sw[i]);
      uart_putc(CONSOLE, sw_states[middle_Sw[i]]);
      uart_puts(CONSOLE, "\r\n");
      return; 
    } 
  }
  uart_printf(CONSOLE,"\033[%u;%uH",r + sw_ind, c);
  uart_printf(CONSOLE,"T%u: ", sw_ind);
  uart_putc(CONSOLE, sw_states[sw_ind]);
  uart_puts(CONSOLE, "\r\n");
}
void new_table_row(char prev_changed_s88, char prev_changed_sensor,
                  char s88_id, char sensor_no, 
                  int dist, int time_diff, int *offset){
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW + *offset,TABLECOL);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, prev_changed_s88 + 'A');
  uart_printf(CONSOLE, "%d", prev_changed_sensor);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, s88_id + 'A');
  uart_printf(CONSOLE, "%d", sensor_no);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", dist);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", time_diff);
  uart_putc(CONSOLE, '|');
  uart_puts(CONSOLE, "\r\n");
  *offset += 1;
}

// this would wake up every time a sensor is triggered or a sensor is released or a switch is changed
void switchSensorTrain_Server(){
  RegisterAs("switchSensorTrain_Server");
  struct train train_list[TRAIN_MAX];
  int train_speed[TRAIN_MAX];
  int sensor_pushed[TRAIN_MAX];
  struct track_node *track;
  init_tracka(track);
  struct track_node *trackmap[20][20];
  get_track_node_map(track, trackmap);

  char sw_states[SWITCH_COUNT];
  // define the previouse changed sensor, switch, state
  char prev_changed_s88 = -1;
  char prev_changed_sensor = -1;
  char prev_changed_switch = -1;
  struct train cur_train;
  int offset = 0;
  // set the pointer to the correct location for table qith TABLEROW and TABLECOL
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW + offset,TABLECOL);
  /*
  
  | Previous  Sensor | current sensor | Distance traveled | Time Elapsed (Ticks) |
| --- | --- | --- | --- |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
*/
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW - 2,TABLECOL);
  uart_printf(CONSOLE,"| Previous  Sensor | current sensor | Distance traveled | Time Elapsed (Ticks) |\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW - 1 ,TABLECOL);
  uart_printf(CONSOLE,"| --- | --- | --- | --- |\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW,1);
  for (int i = 0; i < 200; i++){
    uart_putc(CONSOLE, '-');
  }
  while (1)
  {
    int clock_server_tid = WhoIs("clock_server");
    int prev_sensor = 0;
    // receive the message from the MCW
    uint32_t tid;
    char msg[4];
    Receive(&tid, msg, 4);
    /*
    For read it is this
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_no = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = s88_id; // the s88 that is triggered
    send_msg[1] = sensor_no; // the sensor that is triggered
    send_msg[2] = is_released; // is the type of update
    send_msg[3] = 0; // 0 means sensor update
    
    For switch it is this
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_no = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = SwitchNo; // the s88 that is triggered
    send_msg[1] = C'/'S'; // the sensor that is triggered
    send_msg[2] = -1; // is the type of update
    send_msg[3] = 1; // 1 means switch update

    For train speed change it is this
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_no = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = trainNO; // the s88 that is triggered
    send_msg[1] = speed; // the sensor that is triggered
    send_msg[2] = -1; // is the type of update
    send_msg[3] = 2; // 2 means switch update
    */
   
   
    char type = msg[3];
    if(type == 0){
      char s88_id = msg[0];
      int sensor_no = msg[1];
      int is_released = msg[2];
      if(prev_changed_s88 == -1 && prev_changed_sensor == -1 && prev_changed_switch == -1){
        
        cur_train.sensor_time = Time(clock_server_tid);
        cur_train.position = get_track_node(trackmap, s88_id, sensor_no);
        // uart_printf(CONSOLE, "TRAIN DETECTED: %s\r\n", cur_train.position->name);
      }else if(cur_train.position != get_track_node(trackmap, s88_id, sensor_no)){
        // void sensor_push(int sen, int arr[], size_t alen)
        sensor_push(s88_id * 17 + sensor_no, sensor_pushed, TRAIN_MAX);
        //void print_sensors(int arr[], size_t alen, unsigned int column, unsigned int row, size_t line) 
        print_sensors(sensor_pushed, TRAIN_MAX, SENSORCOL, SENSORROW, CONSOLE);
        // get elapsed time
        int current_time = Time(clock_server_tid);
        int time_diff = current_time - cur_train.sensor_time;
        // get current track node
        struct track_node *current_node = get_track_node(trackmap, s88_id, sensor_no);
        // get the distance between the current node and the previous node
        int dist = 0;
        struct track_node *next_node = next_sensor_node(sw_states, cur_train.position, &dist);
        // print in the format of prev node, cur node, dist, time, speed
        // uart_printf(CONSOLE, "prev_node: %s, cur_node: %s, dist: %d, time: %d, speed: %d\r\n", cur_train.position->name, current_node->name, dist, time_diff, dist/time_diff);
        // update the current train position
        cur_train.position = current_node;
        cur_train.sensor_time = current_time;
        new_table_row(prev_changed_s88, prev_changed_sensor, s88_id, sensor_no, dist, time_diff, &offset);
      }        
      prev_changed_s88 = s88_id;
      prev_changed_sensor = sensor_no;
      prev_changed_switch = is_released;
    }  
    if(type == 1){
      char switch_no = msg[0];
      char switch_state = msg[1];
      sw_states[switch_no] = switch_state;
      print_switch(switch_no, sw_states, CONSOLE);
    }
    if(type == 2){
      char train_no = msg[0];
      char speed = msg[1];
    }
    Reply(tid, msg, 0);
  }
  
  Exit();
}

