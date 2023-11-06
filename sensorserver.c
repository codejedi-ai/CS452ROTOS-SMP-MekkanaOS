#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "k2rps.h"
#include "k3tests.h"
#include "clockserver.h"
#include "sensorserver.h"
#include "util.h"
#define s88_no 5

struct free_task_list{
  uint32_t data[NUMPROCS];
  uint32_t tail;
  uint32_t size;
};

/*
read_one_s88(char s88_id) would return one byte of data from the s88
each bit in the byte would reflect the state of the sensor


  if (a != 0){
    display = get_i(a);
  }else if(b != 0){
    display = get_i(b) + 8;
  }
*/
uint64_t l2(uint64_t x){
    if (x == 1) return 0;
    return l2(x / 2) + 1;
}
uint64_t get_i(uint64_t x){
    return 8 - l2(x);
}

uint16_t read_one_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, char s88_id){  
    char byte_1 = (192 + s88_id);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
    char a = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
    char b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
    uint16_t ret = (b << 8) + a;
    return ret;
}
uint16_t read_many_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, uint8_t* reta, uint8_t* retb){ 
    char byte_1 = ( 128 + s88_no);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
    for (uint32_t i = 0; i < s88_no; i ++){
      uint8_t a = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
      // uart_printf(CONSOLE, "%x", a);
      uint8_t b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
      uint8_t display = 0;
      *(reta + i) = a;
      *(retb + i) = b;
      // print on col 3 and 4
      // clear the screen
      
      // uart_printf(CONSOLE, "%x\r\n", b);
    }
    
    return 0; // Dummy return
}
// a,b must only contain one bit that is 1
uint8_t triggered_sensor(uint8_t a, uint8_t b){
  uint8_t display = 0;
  if (a != 0){
    display = get_i(a);
  }else if(b != 0){
    display = get_i(b) + 8;
  }
  return display;
}
void helper_send_message_to_server(int my_tid, int sensor_server_tid, uint8_t s88_no_i, uint8_t changed, uint8_t is_b, uint8_t is_released){
    while (changed != 0)
    {
      char send_msg[4];
      int last_set_bit = changed & -changed;
      // get the last bit that is 1 in a
      int sensor_no = get_i(last_set_bit) + 8 * is_b;
      char s88_id = s88_no_i;
      send_msg[0] = s88_id;
      send_msg[1] = sensor_no;
      send_msg[2] = is_released;
      send_msg[3] = 0; // this is function call
      Send(sensor_server_tid, send_msg, 4, send_msg, 0);
      /* code */
      changed = changed - last_set_bit;
    }
}
void sensor_server_monitor(){
  // create sensor_server
  uint32_t io_TXIC_MARKLIN_server_pid;
  uint32_t io_RXIC_MARKLIN_server_pid;
  // uint32_t io_CTS_MARKLIN_server_pid;
  io_TXIC_MARKLIN_server_pid = WhoIs("io_TXIC_MARKLIN_server");
  io_RXIC_MARKLIN_server_pid = WhoIs("io_RXIC_MARKLIN_server");
  // io_CTS_MARKLIN_server_pid = WhoIs("io_CTS_MARKLIN_server");
  // print sensor server monitor is active
  
  RegisterAs("sensor_server_monitors");
  
  int my_tid = MyTid();
  // notify the sensor_server the most recent changes with the track sensors
  // if two sensors changed within the same reading, then return them in the order of the s88 lables
  // this also calls the display task

  char prev_reta[s88_no];
  char prev_retb[s88_no];
  uart_printf(CONSOLE, "\033[32m");
  uart_printf(CONSOLE, "sensor_server_monitor: is active\r\n");
  // print in white
  uart_printf(CONSOLE, "\033[37m");
  uint64_t sensor_prev = read_many_s88(io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, &prev_reta, &prev_retb);
  // set cursor location to i, j
  // print in green

  while(1){
    char reta[s88_no];
    char retb[s88_no];
    uint32_t get_time_local = get_timerLO();
    // begin to have putC to the marklin would only awake after the marklin has sent all the data
    uint32_t outchar = read_many_s88(io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, &reta, &retb);
    uart_printf(CONSOLE, "\033[2K");
    // all of the sensors are read, 
    // now determine which bits have changed, from 1 to 0 or from 0 to 1
    // and print them out
    for (uint32_t i = 0; i < s88_no; i ++){
      uint8_t a = reta[i];
      uint8_t b = retb[i];
      uint8_t prev_a = prev_reta[i];
      uint8_t prev_b = prev_retb[i];
      // calculate the sensors that are triggered bit by bit
      uint8_t triggered_a = a & (~prev_a);
      //get the last bit that is 1 in a
      uint8_t relased_a = prev_a & (~a);
      uint8_t triggered_b = b & (~prev_b);
      uint8_t relased_b = prev_b & (~b);
      int sensor_server_tid = WhoIs("sensor_server");
      helper_send_message_to_server(my_tid, sensor_server_tid, i, triggered_a, 0, 0);
      helper_send_message_to_server(my_tid, sensor_server_tid, i, triggered_b, 1, 0);
      helper_send_message_to_server(my_tid, sensor_server_tid, i, relased_a, 0, 1);
      helper_send_message_to_server(my_tid, sensor_server_tid, i, relased_b, 1, 1);
      prev_reta[i] = a;
      prev_retb[i] = b;
    }
  }
  
  Exit();
}
void sensor_server(){
  RegisterAs("sensor_server");
  struct free_task_list getNextSensor_list;
  struct free_task_list awaitTrigger_list[s88_no][16][2];
  getNextSensor_list.tail = 0;
  getNextSensor_list.size = 0;
  for(int i = 0; i < s88_no; i++){
    for(int j = 0; j < 16; j++){
      awaitTrigger_list[i][j][0].tail = 0;
      awaitTrigger_list[i][j][0].size = 0;
      awaitTrigger_list[i][j][1].tail = 0;
      awaitTrigger_list[i][j][1].size = 0;
    }
  }
  // notify the sensor_server the most recent changes with the track sensors
  // print in green sensor server can run
  uart_printf(CONSOLE, "\033[32m");
  uart_printf(CONSOLE, "sensor_server: is active\r\n");
  // print in white
  uart_printf(CONSOLE, "\033[37m");
  while (1)
  {
    // receive the message from the sensor_server_monitor
    uint32_t tid;
    char msg[4];
    Receive(&tid, msg, 4);
    
    char s88_id = msg[0];
    if(s88_id >= 'A')
      s88_id = s88_id - 'A';
    char sensor_no = msg[1] - 1; // need to minus 1 as the sensor_no is 1 indexed
    char is_released = msg[2];
    char type = msg[3];
    uart_printf(CONSOLE, "Sensor %d:%d is %d\r\n", s88_id, sensor_no, is_released);
    Reply(tid, msg, 0);
    /*
    if(type == 0){
      
      // this is the sensor_server_monitor
      // unblock the entire awaitTrigger_list
      for (int i = 0; i < getNextSensor_list.size; i++)
      {
        msg[0] = msg[0] + 'A';
        int tid = getNextSensor_list.data[i];
        Reply(tid, msg, 4);
      }
      getNextSensor_list.size = 0;
      // unblock all the tasks that are waiting for the sensor to be triggered
      for (int i = 0; i < awaitTrigger_list[s88_id][sensor_no][is_released].size; i++)
      {
        int tid = awaitTrigger_list[s88_id][sensor_no][is_released].data[i];
        Reply(tid, msg, 4);
      }
      awaitTrigger_list[s88_id][sensor_no][is_released].size = 0;

      
      continue;
    }
    if(type == 1){
      // this is a function call
      // add the tid to the circular list
      getNextSensor_list.data[getNextSensor_list.tail] = tid;
      getNextSensor_list.tail = (getNextSensor_list.tail + 1) % NUMPROCS;
      continue;
    }
    if(type == 2){
      // this is a function call
      // add the tid to the circular list
      awaitTrigger_list[s88_id][sensor_no][is_released].data[awaitTrigger_list[s88_id][sensor_no][is_released].tail] = tid;
      awaitTrigger_list[s88_id][sensor_no][is_released].tail = (awaitTrigger_list[s88_id][sensor_no][is_released].tail + 1) % NUMPROCS;
      continue;
    }
    // print the sensor that is triggered
    // print in green at row 1 and column 0
    // hide the cursor
    // 
*/
  }
  
  Exit();
}
// outfacing methods
// getNextSensor would return the next sensor that is triggered
// 0th byte is the s88_id
// 1st byte is the sensor_no
int getNextSensor(int sensor_server_tid){
  char send_msg[4];
  send_msg[0] = 0;
  send_msg[1] = 0;
  send_msg[2] = 0;
  send_msg[3] = 1; // this is function call
  Send(sensor_server_tid, send_msg, 4, send_msg, 4);
  return *send_msg;
}
// getNextSensor would return the next sensor that is triggered
// state is 0 for triggered, 1 for released
// for s88_id can use A, B, C, D, E or 0 to 4
// for sensor_no can use 1 to 16
int awaitTrigger(int sensor_server_tid, int s88_id, int sensor_no, int state){
  char send_msg[4];
  send_msg[0] = s88_id;
  send_msg[1] = sensor_no;
  send_msg[2] = state;
  send_msg[3] = 2; // this is function call
  Send(sensor_server_tid, send_msg, 4, send_msg, 4);
  return *send_msg;
}