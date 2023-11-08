#include "../rpi.h"
#include "../util.h"
#include "../ioserver.h"
#include "../clockserver.h"
#include "../custstr.h"
#include "../custstr.h"
#include "train.h"
#include "speed_measuring.h"
static const size_t COMMANDMAX_LEN = 64;
#define UNINT_MAX 0xffffffff
#define OVERFLOW_MINUTES = (UNINT_MAX / 1e6) / 60;
#define OVERFLOW_SECONDS = UNINT_MAX / 1e6;
#define OVERFLOW_TENTH_OF_SECOND = UNINT_MAX / 1e5;
#define TOP_ROW 4
#define LEFT_COL 1
#define WINDOW_HEIGHT 39
#define WINDOW_WIDTH 90
#define COMMAND_ROW 41
#define SW_ROW 1
#define MARKLIN_ROW 1
#define SENSORS_ROW 1
#define ACTIVATED_SWITCHES_ROW 9
#define SECOND_COL 16
#define THIRD_COL 48
#define FOURTH 1
#define POLL_TIME 150000
#define SENSOR_LIST_MAXLEN 100
#define QUEUE_MAX_LEN 200
#define SWITCH_COUNT 18
#define ERROR_ROW COMMAND_ROW + 1
#define QUEUE_MAX_ROW COMMAND_ROW + 2
#define SENSOR_QUERRY COMMAND_ROW + 3
// 240 bytes per second
#define S88_NOS 5
void print_table_headers(){
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
  /*
  #define SENSORROW 50
#define SENSORCOL 0
  */
  uart_printf(CONSOLE,"\033[%u;%uH",SENSORROW - 2,SENSORCOL);
  uart_printf(CONSOLE,"Sensor Table\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",SENSORROW - 1,SENSORCOL);
  uart_printf(CONSOLE,"Recent Triggers\r\n");
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW - 3,1);
  for (int i = 0; i < 200; i++){
    uart_putc(CONSOLE, '-');
  }
}
// Serial line 1 on the RPi hat is used for the console
void new_table_row(char prev_changed_s88, char prev_changed_sensor,
                  char s88_id, char sensor_id, 
                  int dist, int time_diff, int *offset){
  uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW + *offset,TABLECOL);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, prev_changed_s88 + 'A');
  uart_printf(CONSOLE, "%d", prev_changed_sensor);
  uart_putc(CONSOLE, '|');
  uart_putc(CONSOLE, s88_id + 'A');
  uart_printf(CONSOLE, "%d", sensor_id);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", dist);
  uart_putc(CONSOLE, '|');
  uart_printf(CONSOLE, "%d", time_diff);
  uart_putc(CONSOLE, '|');
  uart_puts(CONSOLE, "\r\n");
  *offset += 1;
}

void speed_gather(){
  
  // This task prints outputs the speed of the train
  char train_id = 0x01;
  int tid;
  // Receive(&tid, &train_id, 1); Reply(tid, NULL, 0);
  char name[10];
  // register task as TRAIN%d
  // get the train id as string
  // convert train ID to string
  // convert the int train_id to string
  name[0] = 'T';
  name[1] = 'R';
  ui2a(name + 2, 10, name);
  RegisterAs("speed_gather");
  uint32_t prev_time = Time(WhoIs("clock_server"));
  struct train cur_train;
  // get track from track_server
  int track_server_tid = WhoIs("track_server");
  struct track_node trackmap[TRACK_MAX];
  char track_id = get_track_id(track_server_tid);
  print_table_headers();
  
  if (track_id == 'a')
  {
    init_tracka(trackmap);
  }
  else
  {
    init_trackb(trackmap);
  }

  int offset = 0;

  int prev_changed_s88 = 0;
  int prev_changed_sensor = 0;
  char prev_changed_switch = 0;
  const int col = 200;
  int row = 10;
  while (1)
  {
      /* code */
      // await for a sensor to be triggered
      int track_server_tid = WhoIs("track_server");
      int clock_server_tid = WhoIs("clock_server");
// wait for the most recent sensor to be triggered
      // get the current time



      uint64_t delay_interrupt = await_sensor(track_server_tid);
      // set cursor to row and col
      uart_printf(CONSOLE,"\033[%u;%uH",row,col);
      uart_printf(CONSOLE, "delay_interrupt: 0x%x\r\n", delay_interrupt);
      row += 1;
      /*
      // there is a sensor that is recentlly triggered
      uint32_t time_interrupt = delay_interrupt & 0xFFFFFFFF;
      char *sw_states = &delay_interrupt;
      char s88_id = sw_states[7];
      char sensor_id = sw_states[6];
      int is_released = sw_states[5];
      if(!is_released){
        if(cur_train.position == get_track_node(trackmap, s88_id, sensor_id)){
          // the sensor is pressed
          // this is the part of the code that gets activated when a sensor state changes from 0 to 1
          int current_time = Time(clock_server_tid);
          uint32_t time_diff = current_time - prev_time, dist = 0, isexit = 0;
          prev_time = current_time;
        
          // void sensor_push(int sen, int arr[], size_t alen)
          
          //void print_sensors(int arr[], size_t alen, unsigned int column, unsigned int row, size_t line) 
          struct track_node *current_node = get_track_node(trackmap, s88_id, sensor_id);
          // get elapsed time
          
        
          // get current track node
          struct track_node *prev_node =  cur_train.position;
          
          struct track_node *predict_node =  cur_train.predict_sensor;
          
          uart_printf(CONSOLE,"\033[%u;%uH",TABLEROW + offset, PREDICTNODECOL);
          // cclear the line
          uart_printf(CONSOLE,"\033[K");
          uart_printf(CONSOLE, "prev_node: %s, cur_node: %s, predict_node: %s\r\n", cur_train.position->name, current_node->name, predict_node->name);
          dist = dist_to_node(sw_states, prev_node, current_node);
          // reasons for why predict_node is not the same as current_node
          // run a BFS algorithem from the current node towards the predict node
          // broken sensor
          // broken switch
          // thus need to find the path back to the current_node and figure out what went wrong
          
          struct track_node *previouse_node = get_track_node(trackmap, prev_changed_s88, prev_changed_sensor);
          // get the distance between the current node and the previous node
          
          uart_printf(CONSOLE,"\033[%u;%uH%s",TABLEROW + offset, TABLECOL, cur_train.position->name);
          // print the current train position name
          // 
          // print in the format of prev node, cur node, dist, time, speed
          
          //
          // update the current train position
          // if the name of the pred is not equal the ame of the current
              
          // print in white
          uart_printf(CONSOLE, "\033[37m");
          // train position is the last known train position

          cur_train.position = current_node;
          cur_train.dist_to_next_sensor = 0;
          cur_train.predict_sensor = next_type_node(sw_states, NODE_SENSOR, cur_train.position, &cur_train.dist_to_next_sensor, &isexit);
          cur_train.sensor_time = current_time;
          
          new_table_row(prev_changed_s88, prev_changed_sensor, s88_id, sensor_id, dist, time_diff, &offset);
        }else if(cur_train.position == get_track_node(trackmap, s88_id, sensor_id)){
          // this is the part of the code in which the sensor is released.

        }
      }
      prev_changed_s88 = s88_id;
      prev_changed_sensor = sensor_id;
      prev_changed_switch = is_released;
      */
  }
  Exit();
}