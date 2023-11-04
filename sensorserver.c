#include "traincont.h"
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
#define s88_no 5
struct circular_list{
  uint32_t data[s88_no];
  uint32_t head;
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
      uint8_t b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
      uint8_t display = 0;
      *(reta + i) = a;
      *(retb + i) = b;
      //uart_printf(CONSOLE, "a = 0x%x\r\n", a);
      //uart_printf(CONSOLE, "b = 0x%x\r\n", b);
    }
    
    return 0; // Dummy return
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
  uart_printf(CONSOLE, "sensor_server_monitor is active\r\n");
  RegisterAs("sensor_server_monitors");
  int sensor_server_tid = Create(0, sensor_server);

  // notify the sensor_server the most recent changes with the track sensors
  // if two sensors changed within the same reading, then return them in the order of the s88 lables
  // this also calls the display task

  char prev_reta[s88_no];
  char prev_retb[s88_no];
  uint64_t sensor_prev = read_many_s88(io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, &prev_reta, &prev_retb);
  while(1){
    // clear the screen
    // set cursor to 0,0
    //uart_printf(CONSOLE, "\033[H");
    char reta[s88_no];
    char retb[s88_no];
    uart_printf(CONSOLE, "RUNNING POLLING LOOP\r\n");
    uint32_t get_time_local = get_timerLO();
    // begin to have putC to the marklin would only awake after the marklin has sent all the data
    uint32_t outchar = read_many_s88(io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, &reta, &retb);
    // all of the sensors are read, 
    // now determine which bits have changed, from 1 to 0 or from 0 to 1
    // and print them out
    for (uint32_t i = 0; i < s88_no; i ++){
      uint8_t a = reta[i];
      uint8_t b = retb[i];
      uint8_t display = 0;
      if (a != prev_reta[i]){
        display = get_i(a);
      }else if(b != prev_retb[i]){
        display = get_i(b) + 8;
      }
      if (display != 0){
        uart_printf(CONSOLE, "sensor %d is triggered\r\n", display);
      }
    }
  }
  Exit();
}
void sensor_server(){
  RegisterAs("sensor_server");
  // notify the sensor_server the most recent changes with the track sensors

  Exit();
}