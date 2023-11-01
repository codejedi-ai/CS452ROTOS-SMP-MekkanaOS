#include "traincont.h"
#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "k2TimeTests.h"
#include "k2rps.h"
#include "k3tests.h"
#include "clockserver.h"
#include "sensorserver.h"
void sensor_server_notifier(){
  RegisterAs("sensor_server_notifier");
  int sensor_server_tid = Create(0, sensor_server);

  // notify the sensor_server the most recent changes with the track sensors
  // if two sensors changed within the same reading, then return them in the order of the s88 lables
  uint64_t sensor_prev = 0;
  while(1){
    // clear the screen
    // set cursor to 0,0
    uart_printf(CONSOLE, "\033[H");

    char s88_no = 5;
    uint64_t ret;
    ret = 0;
    uint32_t outchar = read_many_s88(s88_no, &ret);
    // print in green
    for (int i = 0; i < s88_no; i ++){
      char s88_name = 'A' + i;
      uint8_t ret_i_state = (uint8_t)*((uint8_t*)&ret + i);
      uint8_t prev_i_state = (uint8_t)*((uint8_t*)&sensor_prev + i);
      // need to send to the name server |sensor char|old value|new value||
      if (ret_i_state != prev_i_state){
        uint32_t send_message = 0;
        send_message = send_message | (s88_name << 24); // 8 bits for the sensor name
        send_message = send_message | (prev_i_state << 16); // 8 bits for the old value
        send_message = send_message | (ret_i_state << 8); // 8 bits for the new value
        Send(sensor_server_tid, &send_message, sizeof(send_message), 0, 0);
      }
      // 0 means released any non 0 number means the sensor at that number is pressed
    }
    ret = sensor_prev;
  }
  Exit();
}
void sensor_server(){
  RegisterAs("sensor_server");
  // notify the sensor_server the most recent changes with the track sensors
  while (1)
  {
    /* code */
    uint32_t recieved_message = 0;
  }
  
}