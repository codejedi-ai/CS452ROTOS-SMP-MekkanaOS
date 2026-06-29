#include "processes.h"
#include "rpi.h"
#include "asm.h"
#include "syscall.h"
#include "nameserver.h"
#include "custstring.h"
#include "gameserver.h"
#include "systimer.h"
#include "clockserver.h"
#include "util.h"
#include "io_api.h"

#include "config.h"
#if !MARKLIN_HW_UART3
#include "auxuart.h"
#endif

/* Solenoid hold time in ticks (10 ms each). Marklin needs the direction
   command to remain energized long enough for the switch motor to throw
   before the OFF code (32) is sent. ~150 ms is the canonical hold. */
#ifndef SOLENOID_HOLD_TICKS
#define SOLENOID_HOLD_TICKS 15
#endif

/*
 * Marklin byte I/O abstraction.
 *
 * On real Pi 4 hardware (MARKLIN_HW_UART3=1) bytes go through the kernel's
 * interrupt-driven IO server on PL011 UART3.
 *
 * Under QEMU (MARKLIN_HW_UART3=0) UART3 isn't modelled, so we bypass the IO
 * server entirely and poll the BCM2835 AUX mini-UART. The QEMU host pipes
 * those bytes to tools/vhw.py over TCP (see qemu/run.sh).
 *
 * The `tx_pid` / `rx_pid` parameters of the lower-level helpers are unused
 * in QEMU mode; left in place so call-sites stay unchanged.
 */
#if MARKLIN_HW_UART3
  #define MK_PUTC(tx_pid, c)   Putc((tx_pid), MARKLIN, (c))
  #define MK_GETC(rx_pid)      Getc((rx_pid), MARKLIN)
#else
  #define MK_PUTC(tx_pid, c)   ((void)(tx_pid), aux_uart_putc((unsigned char)(c)))
  #define MK_GETC(rx_pid)      ((void)(rx_pid), aux_uart_getc())
#endif

#include "track_server.h"
#include "marklin_worker.h"
#define s88_no 5
#define SENSOR 0
#define TRAIN 1
#define SWITCH 2
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
  MK_PUTC(io_TXIC_MARKLIN_server_pid, byte_1);
  char a = MK_GETC(io_RXIC_MARKLIN_server_pid);
  char b = MK_GETC(io_RXIC_MARKLIN_server_pid);
  uint16_t ret = (b << 8) + a;
  return ret;
}
uint16_t read_many_s88(char io_TXIC_MARKLIN_server_pid, char io_RXIC_MARKLIN_server_pid, uint8_t *reta, uint8_t *retb)
{
  char byte_1 = (128 + s88_no);
  MK_PUTC(io_TXIC_MARKLIN_server_pid, byte_1);
  for (uint32_t i = 0; i < s88_no; i++)
  {
    uint8_t a = MK_GETC(io_RXIC_MARKLIN_server_pid);
    uint8_t b = MK_GETC(io_RXIC_MARKLIN_server_pid);
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
    MK_PUTC(io_TXIC_MARKLIN_server_pid, byte_1);
    MK_PUTC(io_TXIC_MARKLIN_server_pid, byte_2);
}
// SWITCH CONTROL COMMAND
void solonoid_command(int io_TXIC_MARKLIN_server_pid, unsigned char solonoid_id, // Solonoid ID. .
                      unsigned char direction){  // S 33 go straight, C 34 go bent
    char byte_1 = 0;
    char byte_2 = 0;
    if (direction ==  'C')  byte_1 = 34;
    else if (direction ==  'S')  byte_1 = 33;
    else return;
    byte_2 = solonoid_id;
    command_wrapper(io_TXIC_MARKLIN_server_pid, byte_1, byte_2);
    /* Hold the coil energized long enough for the switch to actually throw
       before cutting power. Without this delay the OFF byte arrives within
       a few ms (one frame at 2400 baud) and the switch often fails to flip. */
    Delay(WhoIs("clock_server"), SOLENOID_HOLD_TICKS);
    MK_PUTC(io_TXIC_MARKLIN_server_pid, 32); // this shuts down the solonoid
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











void helper_send_message_to_server(int my_tid, int track_server_tid, uint8_t s88_no_i, uint8_t changed, uint8_t is_b, uint8_t is_released)
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
    send_msg[3] = SENSOR; // 0 means sensor update
    Send(track_server_tid, send_msg, 4, send_msg, 0);
    /* code */
    changed = changed - last_set_bit;
  }
}


void uart_print_coulor(int color){
  // if color is 1 print in green
  // if color is 2 print in yellow
  if (color == 1){
    uart_printf(CONSOLE, "\033[32m");
  }else if (color == 2){
    uart_printf(CONSOLE, "\033[33m");
  }
}
// this is the marklin_worker worker task,
/*
It keeps track of the trains and the sensors
It would notify the track_server when there is a change in the sensors
It would keep track of the positions of the trains
*/
void marklin_worker()
{
  int offset = 0;
  // create track_server
  uint32_t io_TXIC_MARKLIN_server_pid;
  uint32_t io_RXIC_MARKLIN_server_pid;
  // uint32_t io_CTS_MARKLIN_server_pid;

  // io_CTS_MARKLIN_server_pid = WhoIs("io_CTS_MARKLIN_server");
  // print sensor server monitor is active

  RegisterAs("marklin_worker");
  int my_tid = MyTid();
  // notify the track_server the most recent changes with the track sensors
  // if two sensors changed within the same reading, then return them in the order of the s88 lables
  // this also calls the display task
  int col = 0;
  char prev_reta[s88_no];
  char prev_retb[s88_no];
  for (size_t i = 0; i < s88_no; i++)
  {
    /* code */
    prev_reta[i] = 0;
    prev_retb[i] = 0;
  }
    // print in green
  uart_printf(CONSOLE, "\033[32m");
  uart_printf(CONSOLE, "Marklin Worker Initiated\r\n");
  uart_printf(CONSOLE, "\033[37m");
  // set cursor location to i, j
  while (1)
  {
    int track_server_tid = WhoIs("track_server");
    {
      int mk_tid = MarklinServerTid();
      while (mk_tid < 0) { Yield(); mk_tid = MarklinServerTid(); }
      io_TXIC_MARKLIN_server_pid = (uint32_t)mk_tid;
      io_RXIC_MARKLIN_server_pid = (uint32_t)mk_tid;
    }
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
    char type = msg[0];
    char id = msg[1];
    char state = msg[2];
    if(type == SWITCH){
      // this is a function call
      // add the tid to the circular list
      // set the train speed
      offset++;
      solonoid_command(io_TXIC_MARKLIN_server_pid, id, state);
      char send_msg[4];
      char switch_id = id;
      char switch_state = state;
      send_msg[0] = switch_id; // the s88 that is triggered
      send_msg[1] = switch_state; // the sensor that is triggered
      send_msg[2] = -1; // is the type of update
      send_msg[3] = SWITCH; // 2 means switch update
      Send(track_server_tid, send_msg, 4, send_msg, 0);
      Reply(tid, msg, 4);
    }else if(type == TRAIN){
      // this is a function call
      // add the tid to the circular list
      // set the train speed
      // print in green train speed changed
      // PRINTSWITCHROW + offset, PRINTSWITCHCOL
      uart_printf(CONSOLE, "\033[%d;%dH", PRINTSWITCHROW + offset, PRINTSWITCHCOL);
      uart_printf(CONSOLE, "\033[32m");
      uart_printf(CONSOLE, "Train %d speed changed to %d\r\n", id, state);
      uart_printf(CONSOLE, "\033[37m");
      offset++;
      if(offset > 10){
        offset = 0;
        col++;
        col = col % 2;
        uart_print_coulor(col);
      }
      execute_train_command(io_TXIC_MARKLIN_server_pid, id, state);
      char send_msg[4];
      char train_id = id;
      char train_speed = state;
      send_msg[0] = train_id; // the s88 that is triggered
      send_msg[1] = train_speed; // the sensor that is triggered
      send_msg[2] = -1; // is the type of update
      send_msg[3] = TRAIN; // 2 means train update
      Send(track_server_tid, send_msg, 4, send_msg, 0);
      Reply(tid, msg, 4);
    }else if(type == SENSOR){
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
        
        helper_send_message_to_server(my_tid, track_server_tid, i, triggered_a, 0, 0);
        helper_send_message_to_server(my_tid, track_server_tid, i, triggered_b, 1, 0);
        helper_send_message_to_server(my_tid, track_server_tid, i, relased_a, 0, 1);
        helper_send_message_to_server(my_tid, track_server_tid, i, relased_b, 1, 1);

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
int triggerReadmarklin_worker(int marklin_worker_tid){
  char send_msg[4];
  send_msg[0] = SENSOR;
  send_msg[1] = -1;
  send_msg[2] = -1;
  send_msg[3] = -1; // this is function call
  Send(marklin_worker_tid, send_msg, 4, send_msg, 4);
  return *send_msg;
}
// outfacing methods
// getNextSensor would return the next sensor that is triggered
// 0th byte is the s88_id
// 1st byte is the sensor_no


void set_solonoid(int marklin_worker_tid, uint8_t sol_id, char state){
  char send_msg[4];
  send_msg[0] = SWITCH;
  send_msg[1] = sol_id;
  send_msg[2] = state;
  send_msg[3] = -1; // this is function call
  Send(marklin_worker_tid, send_msg, 4, send_msg, 4);
}

void set_train_state(int marklin_worker_tid, uint8_t train_id, char speed){
  char send_msg[4];
  send_msg[0] = TRAIN;
  send_msg[1] = train_id;
  send_msg[2] = speed;
  send_msg[3] = -1; // this is function call
  Send(marklin_worker_tid, send_msg, 4, send_msg, 4);
}
  
void marklin_worker_read_notifier(){
    RegisterAs("marklin_worker_read_notifier");
    /* s88 polling cadence. The Marklin link can't push -- we must send the
       "read s88" command (128+N) and consume 2*N reply bytes to learn sensor
       state. A tight while(1) hammers the 2400 baud line and starves any
       other Marklin TX. 5 ticks (=50ms @ 10ms/tick) is well above the
       sensor's mechanical debounce window and leaves headroom for tr/sw. */
    #ifndef MARKLIN_POLL_TICKS
    #define MARKLIN_POLL_TICKS 5
    #endif
    int cs_tid = -1;
    while (cs_tid <= 0) { cs_tid = WhoIs("clock_server"); if (cs_tid <= 0) Yield(); }
    while (1)
    {
      int MCC_tid = WhoIs("marklin_worker");
      triggerReadmarklin_worker(MCC_tid);
      Delay(cs_tid, MARKLIN_POLL_TICKS);
    }
    Exit();
}

// getNextSensor would return the next sensor that is triggered
// state is 0 for triggered, 1 for released
// for s88_id can use A, B, C, D, E or 0 to 4
// for sensor_no can use 1 to 16
int awaitTrigger(int track_server_tid, int s88_id, int sensor_no, int state){
  char send_msg[4];
  send_msg[0] = s88_id;
  send_msg[1] = sensor_no;
  send_msg[2] = state;
  send_msg[3] = 2; // this is function call
  Send(track_server_tid, send_msg, 4, send_msg, 4);
  return *send_msg;
}

void set_switch(int marklin_worker_tid, uint8_t sw_ind, char state){
    int ret;
    // send the message to the switch server
    char send_msg[4];
    send_msg[0] = sw_ind;
    send_msg[1] = state;
    Send(marklin_worker_tid, send_msg, 4, &ret, 4);
}
char get_switch(int marklin_worker_tid, uint8_t sw_ind){
    int ret;
    // send the message to the switch server
    char send_msg[4];
    send_msg[0] = sw_ind;
    send_msg[1] = 0; // this is function call it 
    Send(marklin_worker_tid, send_msg, 4, send_msg, 4);
    return send_msg[1];
}