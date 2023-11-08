#include "../rpi.h"
#include "../util.h"
#include "../nameserver.h"
#include "../ioserver.h"
#include "../custstr.h"
#include "track_server.h"
#define SENSOR 0
#define TRAIN 1
#define SWITCH 2
#define GET_SENSOR_PUSHED 3 // this gets the sensor pushed array
#define GET_TRACK_ID 4 // this gets the switch states array
#define INIT_TRACK 5
#define INIT_TRAIN 6
#define AWAIT_SENSOR_PUSHED 7
struct track_node* next_type_node(char* sw_states,int type, struct track_node *start_node, int* dist, int *isexit)
{
  // the current node is a sensor node then the for loop would not run
  struct track_node *current_node = start_node;
  int offset = 0;
	while((current_node == start_node)||(current_node->type != type && current_node->type != NODE_EXIT)){
    offset++;
		// if the current position is a switch then we need to consult the switch table. Which is managed by the switch worker
    // uart_printf(CONSOLE, "type: %d next_sensor_node: current_node->type = %d, current_node = %s\n",type, current_node->type, current_node->name);
    // uart_getc(CONSOLE);
    // int buf;
    // scanf("%d", &buf);
		if(current_node->type == NODE_BRANCH){
			char sw_state = sw_states[current_node->num];
			// this is a switch
			// check the switch table
			// if the switch is set to straight then we need to return the straight node
			if(sw_state == 'S'){
				
        *dist += current_node->edge[0].dist;
        current_node = current_node->edge[0].dest;
        // print the distance current_node->edge[0].dist
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
      }
			// if the switch is set to curved then we need to return the curved node
			if(sw_state == 'C'){
				
        *dist += current_node->edge[1].dist;
        current_node = current_node->edge[1].dest;
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[1].dist = %d\n", current_node->edge[1].dist);
			}
		}else{
      
      *dist += current_node->edge[0].dist;
      current_node = current_node->edge[0].dest;
      // print the distance current_node->edge[0].dist
      // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
      // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
    }
  }
  if(current_node->type == NODE_EXIT){
    *isexit = 1;
  }
  // this node can be exit node or not
	return current_node;
}


int dist_to_node(char* sw_states, struct track_node *start_node, struct track_node *end_node){
  int dist = 0;
  // the current node is a sensor node then the for loop would not run
  struct track_node *current_node = start_node;
	while(current_node != end_node){
		// if the current position is a switch then we need to consult the switch table. Which is managed by the switch worker
    // uart_printf(CONSOLE, "dist_to_node: current_node->type = %d, current_node = %s\n",current_node->type, current_node->name);
		if(current_node->type == NODE_BRANCH){
			char sw_state = sw_states[current_node->num];
			// this is a switch
			// check the switch table
			// if the switch is set to straight then we need to return the straight node
			if(sw_state == 'S'){
				
        dist += current_node->edge[0].dist;
        current_node = current_node->edge[0].dest;
        // print the distance current_node->edge[0].dist
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
      }
			// if the switch is set to curved then we need to return the curved node
			if(sw_state == 'C'){
				
        dist += current_node->edge[1].dist;
        current_node = current_node->edge[1].dest;
        // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
        // printf("current_node->edge[1].dist = %d\n", current_node->edge[1].dist);
			}
		}else{
      
      dist += current_node->edge[0].dist;
      current_node = current_node->edge[0].dest;
      // print the distance current_node->edge[0].dist
      // printf("next_sensor_node: next_node->type = %d, next_node = %s\n",current_node->type, current_node->name);
      // printf("current_node->edge[0].dist = %d\n", current_node->edge[0].dist);
    }
  }
  return dist;
}


struct list{
  int await_tids[TRAIN_MAX];
  int await_count;
};
// this would wake up every time a sensor is triggered or a sensor is released or a switch is changed
void track_server(){
  char track_ind = 0;
  RegisterAs("track_server");

  int tid_buf = 0, buf = 0;
  char train_speed[TRAIN_MAX];
  for(int i = 0; i < TRAIN_MAX; i++){
    train_speed[i] = -1;
  }
  char sw_states[SWITCH_COUNT];
  int sensor_pushed[TRAIN_MAX];

  struct track_node track[TRACK_MAX];
  init_tracka(track);
  struct track_node *trackmap[20][20];
  get_track_node_map(track, trackmap);

  
  // define the previouse changed sensor, switch, state
  char prev_changed_s88 = -1;
  char prev_changed_sensor = -1;
  char prev_changed_switch = -1;
  struct train cur_train;
  int offset = 0;
  // set the pointer to the correct location for table qith TABLEROW and TABLECOL
  int first_time = 1;
  struct list await_list;
  await_list.await_count = 0;
  // print in green
  uart_printf(CONSOLE, "\033[32m");
  uart_printf(CONSOLE, "Track Server Initiated\r\n");
  uart_printf(CONSOLE, "\033[37m");
  while (1)
  {
    // uart_printf(CONSOLE, "Track Server: Waiting for message\r\n");
    int clock_server_tid = WhoIs("clock_server");
    int marklin_worker_tid = WhoIs("marklin_worker");
    int prev_sensor = 0;
    // receive the message from the marklin_worker
    uint32_t tid;
    char msg[4];
    Receive(&tid, msg, 4);
    /*
    For read it is this
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_id = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = s88_id; // the s88 that is triggered
    send_msg[1] = sensor_id; // the sensor that is triggered
    send_msg[2] = is_released; // is the type of update
    send_msg[3] = 0; // 0 means sensor update
    
    For switch it is this
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_id = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = SwitchNo; // the s88 that is triggered
    send_msg[1] = C'/'S'; // the sensor that is triggered
    send_msg[2] = -1; // is the type of update
    send_msg[3] = 1; // 1 means switch update

    For train speed change it is this
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_id = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = trainNO; // the s88 that is triggered
    send_msg[1] = speed; // the sensor that is triggered
    send_msg[2] = -1; // is the type of update
    send_msg[3] = SWITCH; // 2 means switch update
    */
   
   
    char type = msg[3];
    if(type == SENSOR){
      // uart_printf(CONSOLE, "SENSOR\r\n");
      
      // unfortunatelly the server does not provide the identity of the train. It would only awake all tasks that are waiting on the sensor_pushed
      
      //input:
      //    send_msg[0] = s88_id; // the s88 that is triggered
      //    send_msg[1] = sensor_no; // the sensor that is triggered
      //    send_msg[2] = is_released; // is the type of update
    
      char s88_id = msg[0];
      char sensor_id = msg[1];
      char is_released = msg[2];
      uint32_t time = Time(clock_server_tid);
      if(!is_released){
        // uart_printf(CONSOLE, "\033[37mSENSOR RELEASED!!\r\n");
        sensor_push(s88_id * 17 + sensor_id, sensor_pushed, TRAIN_MAX);
      }
      // uart_printf(CONSOLE, "\033[37mSWITCH PRESSED!!");
      // reply to all tasks that is waiting on the sensor_pushed
      // tasks would just be waiting on sensor_pushed 
      // every time when this fires we need to return:
      // prev_changed_s88
      // prev_changed_sensor
      // s88_id
      // sensor_id
      // dist
      // time_diff
      // repmsg
      // FREE ALL THE SENSORS
        if(await_list.await_count){
          uint64_t repmsg = time;
          char *repmsg_char = (char*)&repmsg;
          repmsg_char[7] = s88_id;
          repmsg_char[6] = sensor_id;
          repmsg_char[5] = is_released;
          for(int i = 0; i < await_list.await_count; i++){
            Reply(await_list.await_tids[i], &repmsg, 8);
          }
        
        }
      // update the sensor_pushed array
      await_list.await_count = 0;
      
     Reply(tid, &buf, 4);
     // uart_printf(CONSOLE, "SENSOR REPLIED\r\n");
    }  
    if(type == SWITCH){
      // uart_printf(CONSOLE, "SWITCH\r\n");
      // This is called by the worker task
      char switch_no = msg[0];
      char switch_state = msg[1];
      sw_states[switch_no] = switch_state;
      // THIS IS THE ONLY EXCEPTION OF SERVER PRINT
      print_switch(switch_no, (char)switch_state, CONSOLE);
      Reply(tid, msg, 0);
      // uart_printf(CONSOLE, "SWITCH REPLIED\r\n");
    }
    if(type == TRAIN){
      // uart_printf(CONSOLE, "TRAIN\r\n");
      // This is called by the worker task
      char train_no = msg[0];
      char speed = msg[1];

      offset++;
      Reply(tid, msg, 0);
      // uart_printf(CONSOLE, "TRAIN REPLIED\r\n");
    }
    if(type == GET_SENSOR_PUSHED){
      // uart_printf(CONSOLE, "GET_SENSOR_PUSHED\r\n");
      Reply(tid, sensor_pushed, TRAIN_MAX * sizeof(int));
      // uart_printf(CONSOLE, "GET_SENSOR_PUSHED REPLIED\r\n");
    }
    if(type == GET_TRACK_ID){
      // set cursor to tow 20 of col 200

      // struct track_node track[TRACK_MAX];
      Reply(tid, track_ind, 1);
      uart_printf(CONSOLE, "\033[%d;%dH", 21, 200);
      uart_printf(CONSOLE, "GET_TRACK_ID REPLIED\r\n");
    }
    if(type == INIT_TRACK){
      // uart_printf(CONSOLE, "INIT_TRACK\r\n");
      int buf = 1;
      char track_id = msg[0];
      if(track_id == 'a'){
        track_ind = 'a';
        init_tracka(track);
      }
      if(track_id == 'b'){
        track_ind = 'b';
        init_trackb(track);
      }
      Reply(tid, &buf, 4);
      // uart_printf(CONSOLE, "INIT_TRACK REPLIED\r\n");
    }
    if(type == INIT_TRAIN){
      // uart_printf(CONSOLE, "INIT_TRAIN\r\n");
      int buf = 1;
      int train_no = msg[0];
      int s88_id = msg[1];
      int sensor_id = msg[2];
      // init_traina(track, train_no, s88_id, sensor_id);
      Reply(tid, &buf, 4);
      // uart_printf(CONSOLE, "INIT_TRAIN REPLIED\r\n");
    }
    if(type == AWAIT_SENSOR_PUSHED){
      uart_printf(CONSOLE, "\033[%d;%dH", 22, 200);
      uart_printf(CONSOLE, "AWAIT_SENSOR_PUSHED\r\n");
      // add the tid to the list
      // if the list is empty then we need to awit for the sensor to be pushed
      await_list.await_tids[await_list.await_count] = tid;
      await_list.await_count++;
    }
  }
  Exit();
}

void get_sensor_pushed(int track_server_tid, struct track_node track[TRACK_MAX], int tracks_max){
  // this is primarily for the purpose of
  // print_sensors(sensor_pushed, 10, SENSORCOL, SENSORROW, CONSOLE);
  char buf[4];
  buf[0] = -1;
  buf[1] = -1;
  buf[2] = -1;
  buf[3] = GET_SENSOR_PUSHED;
  Send(track_server_tid, &buf, 4, track, sizeof(int) * tracks_max);
}
char get_track_id(int track_server_tid){
  char buf[4];
  buf[0] = -1;
  buf[1] = -1;
  buf[2] = -1;
  buf[3] = GET_TRACK_ID;
  char ret = 0;
  // uart_printf(CONSOLE, "\033[%d;%dH", 20, 200);
  // uart_printf(CONSOLE, "GET_TRACK_ID\r\n");
  Send(track_server_tid, &buf, 4, &ret, 1);
  return ret;
}
void init_track(int track_server_tid, char track_id){
  char buf[4];
  buf[0] = track_id;
  buf[1] = -1;
  buf[2] = -1;
  buf[3] = INIT_TRACK;
  Send(track_server_tid, buf, 4, buf, 4);
}
void init_train(int track_server_tid, char train_no, char s88_id, char sensor_id){
  // the sensor node must be at a START NODE is to be called when a train task is created
  char buf[4];
  buf[0] = train_no;
  buf[1] = s88_id;
  buf[2] = sensor_id;
  buf[3] = INIT_TRAIN;
  // this would call the marklin worker to send the train speed to 0 in addition to setting up the train on the server
  // I have s88 and need to initialize the train on an entry node
  Send(track_server_tid, buf, 4, buf, 4);
  // return -1 if the task is declined
}
void deregister_train(int track_server_tid, char train_no){
  // the sensor node must be at a START NODE is to be called when a train task is created
  // to be completed
}
uint64_t await_sensor(int track_server_tid){
  //  char s88_id, char sensor_id, char is_released
  char buf[4];
  buf[0] = -1;
  buf[1] = -1;
  buf[2] = -1;
  buf[3] = AWAIT_SENSOR_PUSHED;
  uint64_t repmsg = 0;
  Send(track_server_tid, buf, 4, &repmsg, 8);
  return repmsg;
}