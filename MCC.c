#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstr.h"
#include "gameserver.h"
#include "systimer.h"
#include "clockserver.h"
#include "trainnsol.h"
#include "MCC.h"
#define s88_no 5
struct circular_list
{
  uint32_t data[s88_no];
  uint32_t tail;
  uint32_t size;
};
uint64_t l2(uint64_t x)
{
  if (x == 1)
    return 0;
  return l2(x / 2) + 1;
}
uint64_t get_i(uint64_t x)
{
  return 8 - l2(x);
}

/*
read_one_s88(char s88_id) would return one byte of data from the s88
each bit in the byte would reflect the state of the sensor


  if (a != 0){
    display = get_i(a);
  }else if(b != 0){
    display = get_i(b) + 8;
  }
*/
// READ CONTROL COMMAND
uint16_t read_one_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, char s88_id)
{
  char byte_1 = (192 + s88_id);
  Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
  char a = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
  char b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
  uint16_t ret = (b << 8) + a;
  return ret;
}
uint16_t read_many_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, uint8_t *reta, uint8_t *retb)
{
  char byte_1 = (128 + s88_no);
  Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
  for (uint32_t i = 0; i < s88_no; i++)
  {
    uint8_t a = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
    uint8_t b = Getc(io_RXIC_MARKLIN_server_pid, MARKLIN); // would only return if interrupt is recieved
    uint8_t display = 0;
    *(reta + i) = a;
    *(retb + i) = b;
  }

  return 0; // Dummy return
}
// a,b must only contain one bit that is 1
uint8_t triggered_sensor(uint8_t a, uint8_t b)
{
  uint8_t display = 0;
  if (a != 0)
  {
    display = get_i(a);
  }
  else if (b != 0)
  {
    display = get_i(b) + 8;
  }
  return display;
}

void command_wrapper(int io_TXIC_MARKLIN_server_pid, unsigned char byte_1, unsigned char byte_2 ){
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_2);
}
// SWITCH CONTROL COMMAND
void solonoid_command(int io_TXIC_MARKLIN_server_pid, unsigned char solonoid_id, // Solonoid ID. . 
                      unsigned char direction){  // S 33 go straight, C 34 go bent
    char byte_1 = 0;
    char byte_2 = 0;
    if (direction ==  'C')  byte_1 = 34;  
    else if (direction ==  'S')  byte_1 = 33;
    else{
      print_error("ERROR: INVALID DIRECTION");
      return;
    }
    byte_2 = solonoid_id;
    command_wrapper(io_TXIC_MARKLIN_server_pid, byte_1, byte_2);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, 32); // this shuts down the solonoid
}
char complement (char c){
  if (c == 'C') return 'S';
  else if (c == 'S') return 'C';
  else return 0;
}
char complement_sw (char c){
  if (c == 0x99) return 0x9a;
  else if (c == 0x9a) return 0x99;
  else if (c == 0x9b) return 0x9c;
  else if (c == 0x9c) return 0x9b;
  else return 0;
}
// TRAIN CONTROL

void execute_train_command(int io_TXIC_MARKLIN_server_pid, unsigned char id, // Binary: 00001010 
                           unsigned char speed){  // Binary: 00000001)
      command_wrapper(io_TXIC_MARKLIN_server_pid, speed, id);
}











void helper_send_message_to_server(int my_tid, int switchSensorTrain_Server_tid, uint8_t s88_no_i, uint8_t changed, uint8_t is_b, uint8_t is_released)
{
  while (changed != 0)
  {
    char send_msg[4];
    int last_set_bit = changed & -changed;
    // get the last bit that is 1 in a
    int sensor_no = get_i(last_set_bit) + 8 * is_b;
    char s88_id = s88_no_i;
    send_msg[0] = s88_id; // the s88 that is triggered
    send_msg[1] = sensor_no; // the sensor that is triggered
    send_msg[2] = is_released; // is the type of update
    send_msg[3] = 0; // 0 means sensor update
    Send(switchSensorTrain_Server_tid, send_msg, 4, send_msg, 0);
    /* code */
    changed = changed - last_set_bit;
  }
}






// this is the MCW worker task,
/*
It keeps track of the trains and the sensors
It would notify the switchSensorTrain_Server when there is a change in the sensors
It would keep track of the positions of the trains
*/
void MCW()
{
  // create switchSensorTrain_Server
  uint32_t io_TXIC_MARKLIN_server_pid;
  uint32_t io_RXIC_MARKLIN_server_pid;
  // uint32_t io_CTS_MARKLIN_server_pid;

  // io_CTS_MARKLIN_server_pid = WhoIs("io_CTS_MARKLIN_server");
  // print sensor server monitor is active

  RegisterAs("MCW");
  uart_printf(CONSOLE, "created MCW\r\n");
  int my_tid = MyTid();
  // notify the switchSensorTrain_Server the most recent changes with the track sensors
  // if two sensors changed within the same reading, then return them in the order of the s88 lables
  // this also calls the display task

  char prev_reta[s88_no];
  char prev_retb[s88_no];
  for (size_t i = 0; i < s88_no; i++)
  {
    /* code */
    prev_reta[i] = 0;
    prev_retb[i] = 0;
  }
  
  // set cursor location to i, j
  while (1)
  {
    int switchSensorTrain_Server_tid = WhoIs("switchSensorTrain_Server");
    io_TXIC_MARKLIN_server_pid = WhoIs("io_TXIC_MARKLIN_server");
    io_RXIC_MARKLIN_server_pid = WhoIs("io_RXIC_MARKLIN_server");
      // send command format 4 bytes
      // first byte denote the type of command 0 read,
      // second byte -1 (id byte)
      // third byte -1 (state byte)

      // first byte denote the type of command 1 set train speed
      // second byte is the train id
      // third byte is the train speed
      // first byte is 2 meaning it controls the switch
      // second byte denote the switch id
      // third byte denote the switch direction
      // the worker task would execute the command in addition to updating the servers for the switch and the train speeds
    int tid;
    char msg[4];
    Receive(&tid, msg, 4);
    // uart_printf(CONSOLE, "MCW: tid = %d, msg = %d, %d, %d, %d\r\n", tid, msg[0], msg[1], msg[2], msg[3]);
    char type = msg[0];
    char id = msg[1];
    char state = msg[2];
    
    if(type == 2){
      // this is a function call
      // add the tid to the circular list
      // set the train speed
      solonoid_command(io_TXIC_MARKLIN_server_pid, id, state);
      Reply(tid, msg, 4);
    }else if(type == 1){
      // this is a function call
      // add the tid to the circular list
      // set the train speed
      execute_train_command(io_TXIC_MARKLIN_server_pid, id, state);
      Reply(tid, msg, 4);
    }else if(type == 0){
      // uart_printf(CONSOLE, "MCW: read_many_s88\r\n");
      // READ THE MARKLIN initiate a read
      char reta[s88_no];
      char retb[s88_no];

      //uart_getc(CONSOLE);
      // begin to have putC to the marklin would only awake after the marklin has sent all the data
      uint32_t outchar = read_many_s88(io_TXIC_MARKLIN_server_pid, io_RXIC_MARKLIN_server_pid, &reta, &retb);

      // all of the sensors are read,
      // now determine which bits have changed, from 1 to 0 or from 0 to 1
      // and print them out
      for (uint32_t i = 0; i < s88_no; i++)
      {
        uint8_t a = reta[i];
        uint8_t b = retb[i];
        uint8_t prev_a = prev_reta[i];
        uint8_t prev_b = prev_retb[i];
        // calculate the sensors that are triggered bit by bit
        uint8_t triggered_a = a & (~prev_a);
        // get the last bit that is 1 in a
        uint8_t relased_a = prev_a & (~a);
        uint8_t triggered_b = b & (~prev_b);
        uint8_t relased_b = prev_b & (~b);
        
        helper_send_message_to_server(my_tid, switchSensorTrain_Server_tid, i, triggered_a, 0, 0);
        helper_send_message_to_server(my_tid, switchSensorTrain_Server_tid, i, triggered_b, 1, 0);
        helper_send_message_to_server(my_tid, switchSensorTrain_Server_tid, i, relased_a, 0, 1);
        helper_send_message_to_server(my_tid, switchSensorTrain_Server_tid, i, relased_b, 1, 1);

        prev_reta[i] = a;
        prev_retb[i] = b;
      }
      // okay finnished reading the S88s
      // reply to the task that called read
      Reply(tid, msg, 4);
    }
  }
  Exit();
}



// outfacing methods
// getNextSensor would return the next sensor that is triggered
// 0th byte is the s88_id
// 1st byte is the sensor_no
int triggerReadMCW(int MCW_tid){
  char send_msg[4];
  send_msg[0] = 0;
  send_msg[1] = -1;
  send_msg[2] = -1;
  send_msg[3] = -1; // this is function call
  Send(MCW_tid, send_msg, 4, send_msg, 4);
  return *send_msg;
}
// outfacing methods
// getNextSensor would return the next sensor that is triggered
// 0th byte is the s88_id
// 1st byte is the sensor_no
void set_solonoid(int MCW_tid, uint8_t sol_id, char state){
  char send_msg[4];
  send_msg[0] = 2;
  send_msg[1] = sol_id;
  send_msg[2] = state;
  send_msg[3] = -1; // this is function call
  Send(MCW_tid, send_msg, 4, send_msg, 4);
}

void set_train_speed(int MCW_tid, uint8_t train_id, char speed){
  char send_msg[4];
  send_msg[0] = 1;
  send_msg[1] = train_id;
  send_msg[2] = speed;
  send_msg[3] = -1; // this is function call
  Send(MCW_tid, send_msg, 4, send_msg, 4);
}
  
void MCW_read_notifier(){
    RegisterAs("MCW_read_notifier");
    //int tid;
    //int ret;
    //char sw_states[SWITCH_MAX_count];
    //int io_TXIC_MARKLIN_server_pid =  WhoIs("io_TXIC_MARKLIN_server");
    uart_printf(CONSOLE, "MCW_read_notifier is active\r\n");
    
    while (1)
    {
        int MCC_tid = WhoIs("MCW");
        //uart_printf(CONSOLE, "MCW_read_notifier: WhoIs MCW = %d\r\n", MCC_tid);
        triggerReadMCW(MCC_tid);
    }
    
   //uart_printf(CONSOLE, "switch_worker have exited\r\n");
    Exit();
}

// getNextSensor would return the next sensor that is triggered
// state is 0 for triggered, 1 for released
// for s88_id can use A, B, C, D, E or 0 to 4
// for sensor_no can use 1 to 16
int awaitTrigger(int switchSensorTrain_Server_tid, int s88_id, int sensor_no, int state){
  char send_msg[4];
  send_msg[0] = s88_id;
  send_msg[1] = sensor_no;
  send_msg[2] = state;
  send_msg[3] = 2; // this is function call
  Send(switchSensorTrain_Server_tid, send_msg, 4, send_msg, 4);
  return *send_msg;
}

void set_switch(int MCW_tid, uint8_t sw_ind, char state){
    int ret;
    // send the message to the switch server
    char send_msg[4];
    send_msg[0] = sw_ind;
    send_msg[1] = state;
    Send(MCW_tid, send_msg, 4, &ret, 4);
}
char get_switch(int MCW_tid, uint8_t sw_ind){
    int ret;
    // send the message to the switch server
    char send_msg[4];
    send_msg[0] = sw_ind;
    send_msg[1] = 0; // this is function call it 
    Send(MCW_tid, send_msg, 4, send_msg, 4);
    return send_msg[1];
}