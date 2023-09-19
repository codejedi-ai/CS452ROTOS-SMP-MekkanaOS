# @CS452 - Assignment 0

Due: September 19, 2023 8:30 AM
Excused: No
Days Late: 0
Weighted Grade: 0
Final Grade: 0

### Due Date: Tue Sep 19th, 8:30am

## Introduction

In this assignment you will familiarize yourselves with the basic functionality of the Raspberry Pi, its ARM CPU, and the train controller, while learning techniques for handling asynchronous events by polling. It is recommended that you first write, compile, and execute several simple programs for the Raspberry Pi, and only then start working on the assignment.

## Description

Write a program that runs on the Raspberry Pi system. Your program interacts with the user via the serial interface to a virtual terminal while controlling the following independent real-time activities:

- [x]  a digital clock showing minutes, seconds, and tenths of seconds, which measures time accurately, in the sense that it does not slow down or lose ticks,
- [x]  a user command interface that can be used to set train speeds and control switch turn-outs, and
- [x]  a real-time display on the terminal showing the state of the track.

The monitor should use cursor addressing (using appropriate ASCII sequences for terminal control) to display on its screen at least the following:

- [x]  the current time,
- [x]  a table of switch positions,
- [x]  a list of the most recently triggered sensors, and
- [x]  a prompt at which the user can type commands.

These four components should be organized so that the display is easy to understand.

The user command interface should support, at a minimum, the following commands.

1. tr <train number> <train speed> – Set any train in motion at the desired speed (0 for stop).
2. rv <train number> – The train should reverse direction. To reverse direction set the train speed to zero, wait until the train stops, send the reverse direction command, then send a speed command to re-accelerate the train to the same speed in reverse as it was going forward before the command. Do not send the reverse direction command before the train has stopped!
3. sw <switch number> <switch direction> – Throw the given switch to straight (S) or curved (C). Make sure you can reliably communicate with the track and train hardware before attempting to throw switches.**IMPORTANT**: A solenoid must be switched off (via Command 0x20) between 100 and 500 milliseconds after it was activated! Also, do not under any circumstances activate a switch over and over again. Doing so will burn out the solenoid!
4. q – halt the program and return to boot loader (reboot).

Use the Raspberry Pi's built-in system timer ( [BCM](https://student.cs.uwaterloo.ca/~cs452/docs/rpi4b/bcm2711-peripherals.pdf) Chapter 10) to implement the clock. Do not use interrupts.

## Program Structure

Your program should be written as a polling loop, using output channel 1 to talk to the terminal and channel 2 to talk to the train set. (This is the default configuration and you should always leave it so.) The clock should not lose time.

## Possibly Helpful Comments

You can use the headlight on the trains as an easy method for telling if your commands to the trains are successful. Also, the headlights provide an indication of the current travelling direction of a train.

It should not be necessary to use the Märklin 6021 manual control box other than the 'stop' and 'go' buttons (stop+go = reset).

The Pi-equipped workstations in the main lab have multiple serial ports. You can run two gtkterm terminals on the workstations, with one set to the characteristics of the train interface. In that way you can separate a hard problem, 'Is the problem that the UART transmits incorrectly or that my train commands are incorrect?’ into two simpler problems.

For this assignment only, do not use compiler optimization!

## Submission

The assignment must be done completely individually. Hand in the following, nicely formatted and printed.

1. A description of the structure of your program. We will judge your program primarily on the basis of this description. Describe which algorithms and data structures are used and why you chose them. This description should include a list of unimplemented aspects of the assignments, if any. Also, if you know of bugs in your implementation describe them explicitly.
    1. I made a queue for marklin command execution. The queue is circular in sence that the front and back pointing indicies would always loop back to the front. I made the command execution to consume/swallow commands faster than one can issue them. This way we would not have a huge buildup. 
    2. I also employed another circular list to keep track of which sensors were activated by the train. Since I am only putting 10 recently executed sensors
2. Answers to the following questions:
    1. How do you know that your clock does not miss updates or lose time?
        1. I ensured every polling loop is under 100ms. This way the clock would not skip time. I would also use the system-timer call since the system timer is consistent. 
        2. Ensure the time of EVERY polling loop is less than 100 ms including reads.
3. How long does the train hardware take to reply to a sensor query? (Note: To answer these questions, you need to perform and report some timing of your polling loop.)
    
    [Model Train Control Program](https://www.notion.so/Model-Train-Control-Program-1f438fc35677473f8e243df518035009?pvs=21)
    
    1. According to calculations the Marklin hardware <get data testing and analysis>. From recent tests I recall it takes $\frac{3}{10}$ of a second to read in the full input. The full input is 10 bytes which the marklin may take longer to reply. 
        
        ![image.jpg](@CS452%20-%20Assignment%200%201fdf0b2d5d3b405db9f066c9e391612d/image.jpg)
        
        For the program I have the variable `loop time` in which I keep track of the time it take for my polling loop to Finnish
        
    2. There is a `sensor-query` time on the bottom of my interface. As expected and predicted the sensor query does take around $3/10$ seconds. 
4. You should have a code repository on UW's git server git.uwaterloo.ca containing the source code of your assignment, instructions how to make the executable, and the PDFs containing the descriptions of your program and answers to the questions above.
    - The code for sensor timing measurements must be part of your submission, along with any special instructions to build, load, and run.
    - Code and documentation must remain unmodified after submission until the assignments have been marked.
    - Email the commit SHA to the instructor and TAs before the deadline. The repository must be set to 'Private' with TAs and instructor added as 'Reporter'.
5. Make sure (test!) that the following sequence of operations results in a successful build of your program in the  environment. Make it clear in  where to find the documentation and executable!
    
    linux.student.cs
    
    README.md
    
    ```
    git clone <your repo>
    git checkout <commit hash>
    make
    
    ```
    

Three examples of highly rated documentation from previous years:

- [example 1](https://student.cs.uwaterloo.ca/~cs452/docs/other/a0doc1.pdf)
- [example 2](https://student.cs.uwaterloo.ca/~cs452/docs/other/a0doc2.pdf)
- [example 3](https://student.cs.uwaterloo.ca/~cs452/docs/other/a0doc3.pdf)

```c
#include "rpi.h"

// Serial line 1 on the RPi hat is used for the console
static const size_t CONSOLE = 1;
static const size_t MARKLIN = 2;
static const int baudrate_mark = 2400;
int kmain() {
  char hello[] = "V7 Hello world, this is iotest (" __TIME__ ")\r\nPress 'q' to reboot\r\n";

  // initialize both console and marklin uarts
  uart_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable(MARKLIN, baudrate_mark);

  uart_puts(CONSOLE, hello);
  // uart_puts(MARKLIN, hello);
  unsigned int speed=1;
  uart_printf(CONSOLE, "READY baudrate_mark = %u\r\n",baudrate_mark);

    // Create a char array with byte1 and byte2
  uart_putc(MARKLIN, 96);
  uart_putc(MARKLIN, '\r');

  uart_putc(MARKLIN, 10 + 16);
  uart_putc(MARKLIN, 2);
  uart_putc(MARKLIN, '\r');
  char c = ' ';
  while (c != 'q') {
    c = uart_getc(CONSOLE);
    if (c == '\r') {
      uart_printf(CONSOLE, " READY Train 1 with speed %u \r\n", speed);
      //uart_printf(MARKLIN, " READY Train 1 with speed %u \r\n", speed);
      unsigned char byte1 = speed; // Binary: 00001010
      unsigned char byte2 = 2;  // Binary: 00000001
      uart_putc(MARKLIN, byte1 + 16);
      uart_putc(MARKLIN, byte2);
      uart_putc(MARKLIN, '\r');
    } else {
      uart_putc(CONSOLE, c);
      speed = c - 48;
    }

  }
  uart_puts(CONSOLE, "\r\n");

  // U-Boot displays the return value from main - might be handy for debugging
  // use uart_putc to command the trains. 
  // the trains take in commands from two bytes
  // <this byte controls the actions on the trail>|<this byte controls the ideitity of the train>
  // action step
  // action byte:
  // action = <set to val>
  // stop = 0
  // speed = 1 - 14
  // reverse = 15
  // 
  return 0;
}
```

```c
uart_printf(CONSOLE, " timerlo:%u %u, %u, %u: \r", (get_timerLO() / 60000000) % 60, (get_timerLO() / 1000000) % 60,(get_timerLO() / 1000) % 1000, get_timerLO() % 1000);
```

Trains control

[Solenoids control??](https://www.notion.so/Solenoids-control-fc594a436f84412f82b5f0afb5f715bb?pvs=21) 

[README.md](https://www.notion.so/README-md-c945efa4c7124115ac75666d63c5e0ab?pvs=21)

```c
#include "rpi.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

// Serial line 1 on the RPi hat is used for the console
static const size_t COMMANDMAX_LEN = 64;
#define UNINT_MAX 0xffffffff
#define OVERFLOW_MINUTES = (UNINT_MAX / 1e6) / 60;
#define OVERFLOW_SECONDS = UNINT_MAX / 1e6;
#define OVERFLOW_TENTH_OF_SECOND = UNINT_MAX / 1e5;
#define TOP_ROW 4
#define LEFT_COL 1
#define WINDOW_HEIGHT 45
#define WINDOW_WIDTH 80
#define COMMAND_ROW 40
#define SENSOR_HIST_LEN 100
#define S88_NOS 5
/*
Code	Effect
"\033[0m"	Reset special formatting (such as colour).
"\033[30m"	Black text.
"\033[31m"	Red text.
"\033[32m"	Green text.
"\033[33m"	Yellow text.
"\033[34m"	Blue text.
"\033[35m"	Magenta text.
"\033[36m"	Cyan text.
"\033[37m"	White text.
*/
void read_one_s88(char s88_id){  
    uart_putc(MARKLIN, 192 + s88_id);
}
// read_many_s88 reads in 
void read_many_s88(char s88_no){ 
    uart_putc(MARKLIN, 128 + s88_no);
}
uint32_t sol_on_time= 0;
char sol_is_on = 0;
int trains_speed[81];
char sensor_reading[2][S88_NOS];
char sensor_reading_old[2][S88_NOS];
char recently_triggered_sensors[SENSOR_HIST_LEN]; // marklin number and the history
char recently_triggered_s88[SENSOR_HIST_LEN]; // marklin number and the history
unsigned int sensor_hist_cur_pointer = 0; // the top most sensor trigger aka the most recent
char sw_states[255];
int sensor_in_bytes_ind = 0; 
// the r is the hor offset from the defined window
void print_byte_in_binary(char b){
  for (int i = 7; i >= 0; i --){
    uart_putc(CONSOLE, (char)(((1 << i) & b) + '0'));
  }
}
void print_line_hor(uint32_t r){
  uart_printf(CONSOLE,"\033[%u;%uH",r , 1);
  for (uint32_t i = 0; i < WINDOW_WIDTH; i ++){
    uart_putc(CONSOLE, '-');
  }
}
void print_line_numbers_hor(uint32_t r){
  uart_printf(CONSOLE,"\033[%u;%uH",r,1);
  for (uint32_t i = 0; i < WINDOW_HEIGHT; i ++){
    uart_printf(CONSOLE,"%u ", i);
  }
}
// the c is the col offset from the defined window
void print_line_ver(uint32_t c){
  for (uint32_t i = 0; i < WINDOW_HEIGHT; i ++){
    uart_printf(CONSOLE,"\033[%u;%uH",i + TOP_ROW,c);
    uart_putc(CONSOLE, '|');
  }
}
void print_line_numbers_ver(uint32_t c){
  for (uint32_t i = 0; i < WINDOW_HEIGHT; i ++){
    uart_printf(CONSOLE,"\033[%u;%uH",i + TOP_ROW,c);
    uart_printf(CONSOLE,"%u", i);
  }
}
void print_sw_states(uint32_t r, uint32_t c){
  uart_printf(CONSOLE,"\033[%u;%uH",r,c);
  for (uint32_t i = 1; i <= 18; i ++){
    uart_printf(CONSOLE,"\033[%u;%uH",r + i - 1, c);
    uart_printf(CONSOLE,"T%u: ", i);
    uart_putc(CONSOLE, sw_states[i]);
    uart_puts(CONSOLE, "\r\n");
  } 
  char middle_Sw[] ={0x99,0x9a,0x9b,0x9c};
  for (uint32_t i = 0; i < 4; i ++){
    char sw_ind = middle_Sw[i];
    uart_printf(CONSOLE,"\033[%u;%uH",r + i + 18, c);
    uart_printf(CONSOLE,"T%x: ", (int) sw_ind);
    uart_putc(CONSOLE, sw_states[(int) sw_ind]);
    uart_puts(CONSOLE, "\r\n");
  } 
}
char streq(const char *str, const char *str_2){
    char ret = 0; 
    while (*str != '\0' && *str_2 != '\0') { // loop until the end of the array
        if (*str != *str_2) return ret;
        str++; // move to the next character
        str_2++; // move to the next character
    }
    // check the edge case in which the string are of different length. WIll terminate at the shorter string
    if (*str != *str_2) return ret;
    ret = 1;
    return ret;
}
void read_marklin(uint32_t s88_unit, char byte_no){
    sensor_reading_old[byte_no][s88_unit] = sensor_reading[byte_no][s88_unit];
    sensor_reading[byte_no][s88_unit] =  uart_getc_modified(MARKLIN);
}
void print_marklin(int r, int c){
    uart_printf(CONSOLE,"\033[%u;%uH",r,c);
    for(int i = 1; i <= S88_NOS; i ++) {
      uart_printf(CONSOLE,"\033[%u;%uH",r + i - 1,c);
      uart_putc(CONSOLE, (char)('A' + i - 1));
      uart_putc(CONSOLE, ':');
      print_byte_in_binary(sensor_reading[0][i]);
      uart_putc(CONSOLE, ' ');
      print_byte_in_binary(sensor_reading[1][i]);
      uart_puts(CONSOLE, "\r\n");
    }
}

/*
char recentlly_triggered() returns a byte that would be the difference between byte1 qand byte 2
byte1 is the old data
byte2 is the new data

if a bit in byte2 is 1 and the bit in byte1 is 0 the returned difference must be a 1
truth table
byte1 byte 1
0 0 0
0 1 1
1 0 0
1 1 0
by binary arithmatic it is just negation of byte 1 and byte 2
*/
char byte_differences(char byte1, char byte2){
  return ((~byte1) & byte2);
}
void print_activated(int r, int c){
    uart_printf(CONSOLE,"\033[%u;%uH",r,c);
    for(int i = 1; i <= S88_NOS; i ++) {
      uart_printf(CONSOLE,"\033[%u;%uH",r + i - 1,c);
      uart_putc(CONSOLE, (char)('A' + i - 1));
      uart_putc(CONSOLE, ':');

      

      char byte_1_diff = byte_differences(sensor_reading_old[0][i], sensor_reading[0][i]);
      char byte_2_diff = byte_differences(sensor_reading_old[1][i], sensor_reading[1][i]);

      print_byte_in_binary(byte_1_diff);
      uart_putc(CONSOLE, ' ');
      print_byte_in_binary(byte_2_diff);
      uart_puts(CONSOLE, "\r\n");
    }
}
void update_the_triggered_sensors(uint16_t s88_module_no){
  char byte_1_old = sensor_reading[0][s88_module_no];
  char byte_2_old = sensor_reading[1][s88_module_no];
  read_one_s88(s88_module_no);
  sensor_reading[0][s88_module_no] =  uart_getc_modified(MARKLIN);
  sensor_reading[1][s88_module_no] =  uart_getc_modified(MARKLIN);
  char byte_1_diff = byte_differences(byte_1_old, sensor_reading[0][s88_module_no]);
  char byte_2_diff = byte_differences(byte_2_old, sensor_reading[1][s88_module_no]);
  char sensor_no = 1;
  // (char)(((1 << i) & b) + '0')

  // update the triggered sensors
  for (int i = 7; i >= 0; i ++ ){
    if((char)(((1 << i) & byte_1_diff) + '0')){
      sensor_hist_cur_pointer = (sensor_hist_cur_pointer + 1) % SENSOR_HIST_LEN;
      recently_triggered_sensors[sensor_hist_cur_pointer] = sensor_no; 
      recently_triggered_s88[sensor_hist_cur_pointer] = s88_module_no;
      sensor_no++;
    }
  }

  for (int i = 7; i >= 0; i ++ ){
    if((char)(((1 << i) & byte_2_diff) + '0')){
      sensor_hist_cur_pointer = (sensor_hist_cur_pointer + 1) % SENSOR_HIST_LEN;
      recently_triggered_sensors[sensor_hist_cur_pointer] = sensor_no; 
      recently_triggered_s88[sensor_hist_cur_pointer] = s88_module_no;
      sensor_no++;
    }
  }
}
/*
void print_updated_sensors(int r, int c){
  
  unsigned int sensor_hist_cur_pointer_old = sensor_hist_cur_pointer + 1;
  for (char i = 1; i <= S88_NOS; i++){
    update_the_triggered_sensors(i);
  }
  int row_offset = 0;
  while (sensor_hist_cur_pointer_old != sensor_hist_cur_pointer){
      uart_printf(CONSOLE,"\033[%u;%uH",r + row_offset, c);
      row_offset++;
      uart_printf(CONSOLE,"%x %x", recently_triggered_s88[sensor_hist_cur_pointer], recently_triggered_sensors[sensor_hist_cur_pointer]);
      uart_puts(CONSOLE, "\r\n");
      sensor_hist_cur_pointer_old = (sensor_hist_cur_pointer_old + 1) % SENSOR_HIST_LEN;
  }
  
  read_marklin();
}
*/
void print_ui_box(){
  // define the rows and cols
  /*
  ----------------------------------------------
  TRACK SENSORS

  */
  uart_printf(CONSOLE,"\033[2J"); 
  print_line_hor(TOP_ROW);

  uart_printf(CONSOLE,"\033[%u;%uH", TOP_ROW + 1, LEFT_COL + 1);
  uart_puts(CONSOLE, "SW");
  print_sw_states(TOP_ROW + 2, LEFT_COL + 1);

  uart_printf(CONSOLE,"\033[%u;%uH", TOP_ROW + 1, LEFT_COL + 16 + 1);
  uart_puts(CONSOLE, "MARKLIN SWITCHE STATES");      
  print_marklin(TOP_ROW + 2, LEFT_COL + 16 + 1);

  uart_printf(CONSOLE,"\033[%u;%uH", TOP_ROW + 1, LEFT_COL + 48 + 1);
  uart_puts(CONSOLE, "ACTIVATED SWITCHES");
  print_activated(TOP_ROW + 2, LEFT_COL + 48 + 1);
  
  print_line_hor(TOP_ROW );
  print_line_hor(TOP_ROW + WINDOW_HEIGHT);
  
  uart_puts(CONSOLE, "\033[34m");
  print_line_ver(LEFT_COL);
  print_line_ver(LEFT_COL + WINDOW_WIDTH);
  uart_puts(CONSOLE, "\033[37m");
  
  print_line_ver(LEFT_COL + 16);
  print_line_ver(LEFT_COL + 48);
  

  // Print all train control and speed

  // Print all switch states

  // Print the track readings

  // 2400 

}
// this box prints the command in typing to the prompt
// Prints the command at COMMAND_ROW
void print_typing_command(char *command){
  uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + COMMAND_ROW,1);
  uart_printf(CONSOLE,"\033[K");  
  uart_puts(CONSOLE, command);
}

void print_error(){
  uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + COMMAND_ROW + 1,1);
  uart_printf(CONSOLE,"\033[K");  
  uart_printf(CONSOLE,"\033[31m"); // "\033[31m" Set the shit to white
  uart_puts(CONSOLE, "ERROR: COMMAND LENGTH EXCEEDED\r\n"); 
  uart_printf(CONSOLE,"\033[37m"); // "\033[37m"
}

char TENTH_OF_SEC = 0;
void show_timer(const unsigned int hi, const unsigned int lo){
  (void)hi;
  unsigned int minutes = (lo / (unsigned int)1e6) / 60;
  unsigned int seconds = lo / (unsigned int)1e6;
  unsigned int tenth_of_second = lo / (unsigned int)1e5;

  if (TENTH_OF_SEC != tenth_of_second%10){
    uart_printf(CONSOLE,"\033[H");
    uart_printf(CONSOLE,"\033[?25l");
    uart_printf(CONSOLE, "Time:%u:%u:%u", minutes, seconds % 60,tenth_of_second%10);
  }

}
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id){  // Binary: 00000001)
      uart_putc(MARKLIN, speed);
      uart_putc(MARKLIN, id);
      trains_speed[id] = speed;
}
void execute_reverse_command(unsigned char id){  // Binary: 00000001)
      uart_putc(MARKLIN, 0);
      uart_putc(MARKLIN, id);
      uart_putc(MARKLIN, 15);
      uart_putc(MARKLIN, id);
      uart_putc(MARKLIN, trains_speed[id]);
      uart_putc(MARKLIN, id);
}
void solonoid_command(unsigned char solonoid_id, // Solonoid ID. . 
                      unsigned char direction){  // S 33 go straight, C 34 go bent
      
      if (direction ==  'C')  uart_putc(MARKLIN, 34);
      if (direction ==  'S')  uart_putc(MARKLIN, 33);
      uart_putc(MARKLIN, solonoid_id);
      sol_on_time = get_timerLO();
      sol_is_on = 1;
      sw_states[solonoid_id] = direction;
      print_sw_states(TOP_ROW + 2, LEFT_COL + 1);
}
void clear_s88(){
    uart_putc(MARKLIN, 192);
}
// solonoid off
void sol_off(){  // Solonoid ID
    uart_putc(MARKLIN, 32);
}
char str_to_int(char *str){
    char ret = 0; 
    while (*str != '\0') { // loop until the end of the array
        ret = 10 * ret;
        ret += a2d(*str);
        str++; // move to the next character
    }
    return ret;
}
// define a function that takes a char array as a parameter
void parse_char_array(char *arr) {
  
  char *ptr; // pointer to traverse the array
  char *num[100]; // array to store the numbers
  int i = 1; // index for the array
  int used_length = 0;
  num[0] = arr;
  num[1] = arr;
  num[2] = arr;
  ptr = arr; // point to the first element of the array
  while (*ptr != '\0') { // loop until the end of the array
    if (*ptr == ' ') { // check if the character is a space
      *ptr = 0;
      num[i] = ptr + 1; // store the value in the array
      i++; // increment the index
    }
    ptr++; // move to the next character
    used_length++;
  }
  // execute here
  if (num[0][0] == 't' && num[0][1] == 'r'){
    // get train number
    char *train_id_ptr = num[1];
    char train_id = str_to_int(train_id_ptr);

    char *train_command_ptr = num[2];
    char train_speed = str_to_int(train_command_ptr);

    execute_train_command(train_speed, train_id);
  }else if (num[0][0] == 'r' && num[0][1] == 'v'){
    // This command is just a prototype. It gives the user the function of reversing it at a different speed
    // get train number
    char *train_command_ptr = num[1];
    char train_id = str_to_int(train_command_ptr);
    execute_reverse_command(train_id);
  }else if (num[0][0] == 's' && num[0][1] == 'w'){
    char *switch_number = num[1];
    char sol_id = str_to_int(switch_number);
    char switch_state = num[2][0];
    solonoid_command(sol_id,  switch_state);
  } else if (streq(num[0], "clear")){
      print_ui_box();
  } else if(streq(num[0], "read")) {
      read_many_s88(S88_NOS);
      expecting_commands = 1;
  }
}

int kmain() {
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable(MARKLIN, MARKLIN_BR);
  uart_init();

  memset(sw_states, '*', 255);
  print_ui_box();
  // move the cursor to the head
  uart_printf(CONSOLE,"\033[H");
  // clear the screen
  
  char* command = NULL;
  memset(command, 0, COMMANDMAX_LEN);
 
  unsigned int row = 2, col = 1, command_len = 0;
  uart_printf(CONSOLE,"\033[%u;%uH",row,col);
  char hello[] = " uart_getc_modified(MARKLIN) read_marklin() every 200 mils; UPDATE, This is d273liu (" __TIME__ ")\r\nPress 'q' to reboot\r\n";
  uart_puts(CONSOLE, hello);
  
  // initialize both console and marklin uarts
  

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
    
    
  // uart_printf(CONSOLE, "PI[%u]> ", counter++);
  char c = ' ';
  

  
  uint32_t read_time = 0; 
  char expecting_commands = 0; // this is the s_88 the program is going to expect
  char expecting_byte = 0;
  while (c != 'q') {
    if(get_timerLO() - read_time >  200000){
      read_time = get_timerLO();
    }
    if(expecting_commands > 0){
      read_marklin(expecting_commands++, expecting_byte);
      print_marklin(TOP_ROW + 2, LEFT_COL + 16 + 1);
      expecting_byte = expecting_byte  ^ 1;
      if(expecting_commands == S88_NOS){
        expecting_commands = 0;
      }
    }
    if (sol_is_on > 0 && get_timerLO() - sol_on_time > 200000){
      sol_off();
      sol_is_on = 0;
    }
    // uart_putc(MARKLIN, (unsigned char)32);
    show_timer(get_timerHI(), get_timerLO()); 

    c = uart_getc_modified(CONSOLE);
    if (c == '\r') {
      // c is the character the terminal is getting in real time. 
      // I want to print what was displayed. 

      // execute the command 
      // need to parse the command

      
      // need to iterate through the command and get the pointers to the 
      // uart_printf(CONSOLE,"\033[%u;%uH",row + counter,col);
      // uart_printf(CONSOLE, "PI[%u]> ", counter++);
      // uart_puts(CONSOLE, command);
      parse_char_array(command);
      // uart_printf(CONSOLE, "\r\n");
      command_len = 0;
      command[command_len] = 0;
      print_typing_command(command);
    } else if (c == '\b' && command_len > 0){
      command[--command_len] = 0;
      print_typing_command(command);
      // I do not know why I need to print this here
    } else {
        // this part we need to increase the string length. 
        if (c != 0 && c != '\b'){
          if (command_len + 1 < COMMANDMAX_LEN - 1){
            command[command_len] = c;
            command[++command_len] = 0;
            print_typing_command(command);
            // the command changes
            // this would keep printing the command even though the command does not change`
          } else {
            print_error();
          }
        }else{

        }

    }
    
    
    // uart_printf(CONSOLE,"\033[2J");
  }
  uart_puts(CONSOLE, "\r\n");
  
  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}
```