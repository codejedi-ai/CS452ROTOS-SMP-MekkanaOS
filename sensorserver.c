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

*/
uint16_t read_one_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, char s88_id){  
    char byte_1 = (192 + s88_id);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
    uint16_t a = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
    uint16_t b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
    if (a != 0){
      return get_i(a);
    }else if(b != 0){
      return get_i(b) + 8;
    }
    return 0;
}
uint16_t read_many_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, uint8_t* ret){ 
    char byte_1 = ( 128 + s88_no);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
    for (uint32_t i = 0; i < s88_no; i ++){
      uint8_t a = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
      uint8_t b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
      uint8_t display = 0;

      if (a != 0){
        display = get_i(a);
      }else if(b != 0){
        display = get_i(b) + 8;
      }
      *(ret + i) = display;
      //uart_printf(CONSOLE, "a = 0x%x\r\n", a);
      //uart_printf(CONSOLE, "b = 0x%x\r\n", b);
    }
    
    return 0; // Dummy return
}
void sensor_server_notifier(){
  uint32_t io_TXIC_MARKLIN_server_pid;
  uint32_t io_RXIC_MARKLIN_server_pid;
  // uint32_t io_CTS_MARKLIN_server_pid;
  io_TXIC_MARKLIN_server_pid = WhoIs("io_TXIC_MARKLIN_server");
  io_RXIC_MARKLIN_server_pid = WhoIs("io_RXIC_MARKLIN_server");
  // io_CTS_MARKLIN_server_pid = WhoIs("io_CTS_MARKLIN_server");
  RegisterAs("sensor_server_notifier");
  int sensor_server_tid = Create(0, sensor_server);

  // notify the sensor_server the most recent changes with the track sensors
  // if two sensors changed within the same reading, then return them in the order of the s88 lables
  uint64_t sensor_prev = 0;
  while(1){
    // clear the screen
    // set cursor to 0,0
    uart_printf(CONSOLE, "\033[H");

    uint64_t ret;
    ret = 0;
    uint32_t outchar = read_many_s88(io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, &ret);
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
        Send(sensor_server_tid, &send_message, sizeof(send_message), &send_message, 0);
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
    int tid = 0;
    Receive(&tid, &recieved_message, sizeof(recieved_message));
    char s88_name = (char)(recieved_message >> 24);
    uint8_t prev_i_state = (uint8_t)(recieved_message >> 16);
    uint8_t ret_i_state = (uint8_t)(recieved_message >> 8);

  }
  
}