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
#define WINDOW_WIDTH 90
#define COMMAND_ROW 40
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
// 240 bytes per second
#define S88_NOS 5
/*
These are the most essential terminal control sequences that you will need for your train program.

Code	Effect
"\033[2J"	Clear the screen.
"\033[H"	Move the cursor to the upper-left corner of the screen.
"\033[r;cH"	Move the cursor to row r, column c. Note that both the rows and columns are indexed starting at 1.
"\033[?25l"	Hide the cursor.
"\033[K"	Delete everything from the cursor to the end of the line.
These control sequences can help make your program's display more lively.

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

uint32_t sol_on_time= 0;
uint8_t trains_speed[81]; // this is the speed of the train
uint8_t trains_direction[81]; // this is the direction of the train
char sensor_reading[2][S88_NOS];
char sensor_reading_old[2][S88_NOS];
uint8_t recently_triggered_sensors[SENSOR_LIST_MAXLEN]; // marklin number and the history
uint8_t recently_triggered_s88[SENSOR_LIST_MAXLEN]; // marklin number and the history



unsigned int sensor_hist_cur_pointer = 0; // the top most sensor trigger aka the most recent
char sw_states[255];
// the r is the hor offset from the defined window
void print_byte_in_binary(unsigned const char b){
  for (int i = 7; i >= 0; i --){
    if (((1 << i) & b) == 0){
      uart_putc(CONSOLE,'0');
    }else{
      uart_putc(CONSOLE,'1');
    }
    
  }
}
void print_line_hor(uint32_t r){
  uart_printf(CONSOLE,"\033[%u;%uH",r , LEFT_COL);
  for (uint32_t i = 0; i < WINDOW_WIDTH; i ++){
    uart_putc(CONSOLE, '-');
  }
}
void print_line_numbers_hor(uint32_t r){
  uart_printf(CONSOLE,"\033[%u;%uH",r, LEFT_COL);
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
  uart_puts(CONSOLE, "SW");
  for (uint32_t i = 1; i <= SWITCH_COUNT; i ++){
    uart_printf(CONSOLE,"\033[%u;%uH",r + i, c);
    uart_printf(CONSOLE,"T%u: ", i);
    uart_putc(CONSOLE, sw_states[i]);
    uart_puts(CONSOLE, "\r\n");
  } 
  char middle_Sw[] ={0x99,0x9a,0x9b,0x9c};
  for (uint32_t i = 0; i < 4; i ++){
    char sw_ind = middle_Sw[i];
    uart_printf(CONSOLE,"\033[%u;%uH",r + i + SWITCH_COUNT + 1, c);
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
void read_marklin(uint32_t s88_unit, short int byte_no){
    sensor_reading_old[byte_no][s88_unit] = sensor_reading[byte_no][s88_unit];
    sensor_reading[byte_no][s88_unit] =  uart_getc_modified(MARKLIN);
}
void print_marklin(int r, int c, uint8_t i){
    uart_printf(CONSOLE,"\033[%u;%uH",r,c);
    uart_puts(CONSOLE, "RECENT SENSOR");    
    uart_printf(CONSOLE,"\033[%u;%uH",r + i,c);
    uart_putc(CONSOLE, (char)('A' + i - 1));
    uart_putc(CONSOLE, ':');
    print_byte_in_binary(sensor_reading[0][i]);
    uart_putc(CONSOLE, ' ');
    print_byte_in_binary(sensor_reading[1][i]);
    uart_puts(CONSOLE, "\r\n");
    
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
void print_activated(uint16_t r, uint16_t c, uint16_t i){
    uart_printf(CONSOLE,"\033[%u;%uH",r,c);
    uart_puts(CONSOLE, "ACTIVATED SWITCHES");
    uart_printf(CONSOLE,"\033[%u;%uH",r + i,c);
    uart_putc(CONSOLE, (char)('A' + i - 1));
    uart_putc(CONSOLE, ':');

    

    char byte_1_diff = byte_differences(sensor_reading_old[0][i], sensor_reading[0][i]);
    char byte_2_diff = byte_differences(sensor_reading_old[1][i], sensor_reading[1][i]);
    /*
    char byte_1_diff = sensor_reading_old[0][i];
    char byte_2_diff = sensor_reading_old[1][i];
    */
    print_byte_in_binary(byte_1_diff);
    uart_putc(CONSOLE, ' ');
    print_byte_in_binary(byte_2_diff);
    uart_puts(CONSOLE, "\r\n");
    
}
void update_the_triggered_sensors(uint8_t s88_module_no, uint8_t byte_no){
  char byte_1_diff = byte_differences(sensor_reading_old[byte_no][s88_module_no], sensor_reading[byte_no][s88_module_no]);
  char sensor_no = 1; // the oth byte handels senssors 1 - 8 while the 1th byte handel sensors 9 - 16
  if (byte_no == 1){
    sensor_no = 9;
  }
  // update the triggered sensors
  for (int i = 7; i >= 0; i -- ){
    if(((1 << i) & byte_1_diff) > 0){
      // only add if it is different from previous reading. the module number is the s88_module_no and sensor number is sensor_no
      // only add if the module number or sensor number is different from the previous reading
      //uint8_t prev_ind = (sensor_hist_cur_pointer - 1 + SENSOR_LIST_MAXLEN) % SENSOR_LIST_MAXLEN;
      //if (recently_triggered_sensors[prev_ind] != sensor_no || recently_triggered_s88[prev_ind] != s88_module_no){
        recently_triggered_sensors[sensor_hist_cur_pointer] = sensor_no; 
        recently_triggered_s88[sensor_hist_cur_pointer] = s88_module_no;
        sensor_hist_cur_pointer = (sensor_hist_cur_pointer + 1) % SENSOR_LIST_MAXLEN;
      //}
    }
    sensor_no++;
  }
}

void print_updated_sensors(int r, int c){
  // set cursor to r,c
  uart_printf(CONSOLE,"\033[%u;%uH",r,c);
  uart_puts(CONSOLE, "RECENTLY ACTIVATED SENSORS");

  for (char i = 1; i <= S88_NOS * 2; i++){
    uint32_t sensor_hist_cur_pointer_old = (sensor_hist_cur_pointer - i + SENSOR_LIST_MAXLEN) % SENSOR_LIST_MAXLEN;
    // sensor_hist_cur_pointer is the current pointer that points at the next index to be written
    uart_printf(CONSOLE,"\033[%u;%uH",r + i,c);
    uart_putc(CONSOLE, (char)('A' + recently_triggered_s88[sensor_hist_cur_pointer_old] - 1));
    uart_putc(CONSOLE, ':');
    // do the printf for the sensor number in hexadecimal
    if (recently_triggered_sensors[sensor_hist_cur_pointer_old] < 10){
      uart_putc(CONSOLE, '0');
    }
    uart_printf(CONSOLE, "%u", recently_triggered_sensors[sensor_hist_cur_pointer_old]);
    uart_puts(CONSOLE, "\r\n");
  }
}

void print_ui_box(){
  // define the rows and cols
  /*
  ----------------------------------------------
  TRACK SENSORS



  */
  uart_printf(CONSOLE,"\033[2J"); 
  print_line_hor(TOP_ROW);

  
  print_sw_states(TOP_ROW + SW_ROW, LEFT_COL + 1);
  // print the marklin states
  for (uint8_t i = 1; i <= S88_NOS; i++)
  print_marklin(TOP_ROW + MARKLIN_ROW, LEFT_COL + SECOND_COL + 1, i);
  // print the activated switches
  uart_printf(CONSOLE,"\033[%u;%uH", TOP_ROW + 8, LEFT_COL + SECOND_COL + 1);
  // print the recentlly activated sensors
  uart_printf(CONSOLE,"\033[%u;%uH", TOP_ROW + 1, LEFT_COL + THIRD_COL + 1);

  print_updated_sensors(TOP_ROW + SENSORS_ROW, LEFT_COL + THIRD_COL + 1);

  print_line_hor(TOP_ROW );
  print_line_hor(TOP_ROW + WINDOW_HEIGHT);
  
  uart_puts(CONSOLE, "\033[34m");
  print_line_ver(LEFT_COL);
  print_line_ver(LEFT_COL + WINDOW_WIDTH);
  uart_puts(CONSOLE, "\033[37m");
  
  print_line_ver(LEFT_COL + SECOND_COL);
  print_line_ver(LEFT_COL + THIRD_COL);
  



  // Print all train control and speed

  // Print all switch states

  // Print the track readings

  // 2400 



}
// this box prints the command in typing to the prompt
// Prints the command at COMMAND_ROW
void print_typing_command(char *command){
  uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + COMMAND_ROW, LEFT_COL + 1);
  uart_printf(CONSOLE,"\033[K");  
  uart_puts(CONSOLE, command);
}

void print_error(char *error_msg){
  uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + ERROR_ROW,LEFT_COL + 1);
  uart_printf(CONSOLE,"\033[K");  
  uart_printf(CONSOLE,"\033[31m"); // "\033[31m" Set the shit to white
  // print error_msg
  uart_puts(CONSOLE, error_msg);
  
  uart_puts(CONSOLE, "\r\n"); 
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
    // clear row
    uart_puts(CONSOLE, "\033[K");
    uart_printf(CONSOLE, "Time:%u:%u:%u", minutes, seconds % 60,tenth_of_second%10);
  }

}
/*
THESE ARE THE EXECUTE MARKLIN COMMANDS BEGIN


*/
char execution_queue[2][QUEUE_MAX_LEN]; // marklin number and the history
uint32_t execution_queue_begin = 0;
uint32_t execution_queue_end = 0;
uint32_t queue_cur_size = 0;
void enqueue(unsigned char byte_1, unsigned char byte_2 ){
  execution_queue[0][execution_queue_end] = byte_1;
  execution_queue[1][execution_queue_end] = byte_2;
  execution_queue_end = (execution_queue_end + 1) % QUEUE_MAX_LEN;
  queue_cur_size++;
}
void dequeue(){
  if (execution_queue[0][execution_queue_begin] == 33 || execution_queue[0][execution_queue_begin] == 34){
    uint8_t sol_id = execution_queue[1][execution_queue_begin];
    if(execution_queue[0][execution_queue_begin] == 33 ){
      sw_states[sol_id] = 'S';
    }else if(execution_queue[0][execution_queue_begin] == 34 ){
      sw_states[sol_id] = 'C';
    }
    // print the switch state
  }
  if (queue_cur_size != 0){
    queue_cur_size--;
    uart_putc(MARKLIN, execution_queue[0][execution_queue_begin]);
    if (execution_queue[1][execution_queue_begin] != '\r'){
      uart_putc(MARKLIN, execution_queue[1][execution_queue_begin]);
    }
    execution_queue_begin = (execution_queue_begin + 1) % QUEUE_MAX_LEN;
  }
  // If the command is a sononoid command then it is going to be a 2 byte command
  // then update the table

}

// This section is the read in
uint8_t expecting_commands = 0; // this is the s_88 the program is going to expect
uint8_t expecting_byte = 0;
//  reads in 
void read_one_s88(char s88_id){  
    char byte_1 = (192 + s88_id);
    // uart_putc(MARKLIN, byte_1);
    enqueue(byte_1, '\r');
    expecting_commands = s88_id;
    expecting_byte = 0;
}
void read_many_s88(char s88_no){ 
    char byte_1 = ( 128 + s88_no);
    enqueue(byte_1, '\r');
    // uart_putc(MARKLIN, byte_1);
    expecting_commands = 1;
    expecting_byte = 0;
}
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id){  // Binary: 00000001)
      enqueue(speed, id);
      trains_speed[id] = speed;
}
void execute_reverse_command(unsigned char id){  // Binary: 00000001)
      enqueue(0, id);
      enqueue(15, id);
      enqueue(trains_speed[id], id);
}
void sol_off(){  // Solonoid ID
    enqueue(32, '\r');
}
void solonoid_command(unsigned char solonoid_id, // Solonoid ID. . 
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
    enqueue(byte_1, byte_2);
    sol_on_time = get_timerLO();
    sw_states[solonoid_id] = direction;
    sol_off();
}
// the function clears the read memory on the s88. 
void clear_s88(){
    enqueue(192, '\r');
}
// solonoid off

void press_go(){  // Press the GO button on the MARKLIN
    enqueue(96, '\r');
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
char hexstr_to_int(char *str){
    char ret = 0; 
    str++;
    str++;
    while (*str != '\0') { // loop until the end of the array
        ret = 16 * ret;
        ret += a2d(*str);
        str++; // move to the next character
    }
    return ret;
}
/*
THESE ARE THE EXECUTE MARKLIN COMMANDS END


COMMANDS START HERE
*/

// define a function that takes a char array as a parameter
void parse_char_array(char *arr) {
  
  char *ptr; // pointer to traverse the array
  char *num[100]; // array to store the numbers
  int i = 1; // index for the array
  int used_length = 0;
  num[0] = NULL;
  num[1] = NULL;
  num[2] = NULL;
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
    if(switch_number[0] == '0' && switch_number[1] == 'x'){
      // this is a hex number
      char sol_id = hexstr_to_int(switch_number);
      char switch_state = num[2][0];
      // 0x99 and 0x9a cannot be both curved at once
      // if 0x99 is curved then 0x9a must be straight and vice versa
      if(sol_id == 0x99){
        if (switch_state == 'C'){
          solonoid_command(0x9a, 'S');
        }else{
          solonoid_command(0x9a, 'C');
        }
      }
      if (sol_id == 0x9a){
        if (switch_state == 'C'){
          solonoid_command(0x99, 'S');
        }else{
          solonoid_command(0x99, 'C');
        }
      }
      // 0x9b and 0x9c cannot be curved at once
      // if 0x9b is curved then 0x9c must be straight and vice versa
      if (sol_id == 0x9b){
        if (switch_state == 'C'){
          solonoid_command(0x9c, 'S');
        }else{
          solonoid_command(0x9c, 'C');
        }
      }
      if (sol_id == 0x9c){
        if (switch_state == 'C'){
          solonoid_command(0x9b, 'S');
        }else{
          solonoid_command(0x9b, 'C');
        }
      }
      solonoid_command(sol_id,  switch_state);
    } else {
      char sol_id = str_to_int(switch_number);
      char switch_state = num[2][0];
      solonoid_command(sol_id,  switch_state);
    }
  } else if (streq(num[0], "clear")){
      print_ui_box();
      clear_s88();
  } else if(streq(num[0], "read")) {
      read_many_s88(S88_NOS);
  } else {
    print_error("ERROR: INVALID COMMAND");
  }
}

// make a function that tests the time at which the marklin replies
// get the current time stamp, use the uart_getc(MARLKIN) command to get the next byte
// Such have to block and only commence to the rest of the code when the byte is received
// break out of the loop whtn the 10 bytes are recienved
// that is from first module 1 to module 2, each module has 2 bytes
void check_marklin_response(uint8_t r, uint8_t c){
  // clear the screen
  uart_printf(CONSOLE,"\033[2J");
  // initialize the expecting command and byte
  expecting_commands = 1;
  expecting_byte = 0;
  // read the marklin
  read_many_s88(S88_NOS);
  // dequeue the marklin
  uint32_t execution_time = get_timerLO();
  
  while (execution_queue_begin != execution_queue_end)
  {
    if(get_timerLO() - execution_time >  POLL_TIME){
      // execute from the queue
      dequeue();
      execution_time = get_timerLO();
    }
  }
  uint32_t timer = get_timerLO();
  uint32_t timer_old = get_timerLO();
  // print prompt to check marlin
  uart_printf(CONSOLE,"\033[%u;%uH",r, c);
  uart_puts(CONSOLE, "CHECKING MARKLIN TIME RESPONSE\r\n");
  while (expecting_commands <= S88_NOS) // the expecting commands is one indexed
  {
    uart_printf(CONSOLE,"\033[%u;%uH", r + (expecting_commands - 1) * 2 + expecting_byte + 1, c);
    /* code */
    // increment expecting byte if it equals 2 increment expecting commands and set expecting byte to 0
    // This is to be ran before the ui is printed
    // if the marklin has a byte to be read
    // this byte would be busywaiting until the byte is read
    char c = uart_getc(MARKLIN);
    // print the byte and the time it taken to read the byte
    // print string byte: 
    // read_marklin(expecting_commands, expecting_byte); 
    // read_marklin(uint32_t s88_unit, short int byte_no)
    sensor_reading_old[expecting_byte][expecting_commands] = sensor_reading[expecting_byte][expecting_commands];
    sensor_reading[expecting_byte][expecting_commands] =  c;


    update_the_triggered_sensors(expecting_commands, expecting_byte);
    uart_puts(CONSOLE, "byte: ");
    // print the byte in the binary form
    print_byte_in_binary(c);
    // print the expecting command and byte
    uart_printf(CONSOLE, " expecting: %u %u ", expecting_commands, expecting_byte);
    uart_printf(CONSOLE, " time taken: %u \r\n", get_timerLO() - timer);
    timer = get_timerLO();
    expecting_byte++;
    if (expecting_byte == 2){
      expecting_commands++;
      expecting_byte = 0;
    }
  }
  // print total time taken to complete
  uart_printf(CONSOLE,"\033[%u;%uH", r + (expecting_commands - 1) * 2 + expecting_byte + 2, c);
  uart_printf(CONSOLE, "total time: %u", get_timerLO() - timer_old);
}
int kmain() {
  
  // INITIALIZE THE VALUES
  execution_queue_begin = 0;
  execution_queue_end = 0;
  queue_cur_size = 0;
  for (int i = 0; i < QUEUE_MAX_LEN; i++){
    execution_queue[0][i] = 0;
    execution_queue[1][i] = 0;
  }
  sensor_hist_cur_pointer = 0;
  for (int i = 0; i < SENSOR_LIST_MAXLEN; i++){
    recently_triggered_sensors[i] = 0;
    recently_triggered_s88[i] = 0;
  }
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable(MARKLIN, MARKLIN_BR);
  uart_init();
  
  // the earliest in which you can print
  
  print_sw_states(TOP_ROW + SW_ROW, LEFT_COL + 1);

  memset(sw_states, '*', 255);
  
  // move the cursor to the head
  uart_printf(CONSOLE,"\033[H");
  // clear the screen
  char* command = NULL;
  memset(command, 0, COMMANDMAX_LEN);


  // press_go();
  check_marklin_response(TOP_ROW + SW_ROW, LEFT_COL + SECOND_COL + 1);

    // make an int array of trains nubmber 1, 2, 24, 47, 54, 58
  char train_numbers[] = {1, 2, 24, 47, 54, 58};
  int train_count = 6;
  // set all the turnabouts to straight
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'S');
    // this command only enqueues the switches
  }
  // uart_printf(CONSOLE,"\033[%u;%uHSWITCHES ALL STRAIGHT:",TOP_ROW + COMMAND_ROW, LEFT_COL + 1);
  // set all the turnabouts to curved
  for (uint8_t i = 1; i <= SWITCH_COUNT; i ++){
    solonoid_command(i, 'C');
  }
  // set all train speed to 0
  for (uint8_t i = 0; i < train_count; i ++){
    execute_train_command(0, train_numbers[i]);
  }
  solonoid_command(0x99, 'S');
  solonoid_command(0x9a, 'C');
  solonoid_command(0x9b, 'S');
  solonoid_command(0x9c, 'C');

  solonoid_command(0x99, 'C');
  solonoid_command(0x9a, 'S');
  solonoid_command(0x9b, 'C');
  solonoid_command(0x9c, 'S');


  uint32_t execution_time = 0;
  char c = ' ', animation_state = 0;
  
  while (execution_queue_begin != execution_queue_end)
  {
    if(get_timerLO() - execution_time >  POLL_TIME){
      
      // execute from the queue
      dequeue();
      print_sw_states(TOP_ROW + SW_ROW, LEFT_COL + 1);
      uart_printf(CONSOLE,"\033[%u;%uHINITIALIZING:",TOP_ROW + COMMAND_ROW, LEFT_COL + 1);
      if (animation_state == 0){
        uart_puts(CONSOLE, "-");
        animation_state = 1;
      }else if (animation_state == 1){
        uart_puts(CONSOLE, "\\");
        animation_state = 2;
      }else if (animation_state == 2){
        uart_puts(CONSOLE, "|");
        animation_state = 3;
      } else if (animation_state == 3){
        uart_puts(CONSOLE, "/");
        animation_state = 0;
      } 
      
      // print the lft and right pointer of the queue
      uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + COMMAND_ROW + 3, 2);
      uart_printf(CONSOLE,"\033[K");
      uart_printf(CONSOLE, "queue: %u %u queue_cur_size: %u", execution_queue_begin, execution_queue_end, queue_cur_size);
      // set coulor to white
      uart_puts(CONSOLE,"\033[37m");
      execution_time = get_timerLO();
    }
  }
  print_ui_box();
  unsigned int row = 2, col = 1, command_len = 0;
  uart_printf(CONSOLE,"\033[%u;%uH",row,col);
  char hello[] = "VLOG a0: This is d273liu (" __TIME__ ")\r\nPress 'q' to reboot\r\n";
  uart_puts(CONSOLE, hello);
  


  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
    
    
  // uart_printf(CONSOLE, "PI[%u]> ", counter++);
  uart_printf(CONSOLE,"\033[%u;%uHENTER COMMAND AT BOTTOM:",TOP_ROW + COMMAND_ROW - 1, LEFT_COL + 1);
  uart_printf(CONSOLE,"\033[%u;%uHFINISHED!!!!!",TOP_ROW + COMMAND_ROW, LEFT_COL + 1);
  command_len = 0;
  
  expecting_commands = 0; // this is the s_88 the program is going to expect
  
  // INITIALIZE THE TURNOUTS
  
  uint32_t loop_time = get_timerLO(), max_loop_time = 0;
  while (c != 'q') {
    show_timer(get_timerHI(), get_timerLO()); 
    loop_time = get_timerLO();
    if(get_timerLO() - execution_time >  POLL_TIME){
      dequeue();
      if(!uart_getc_queue(MARKLIN) && expecting_commands == 0){
        // not reading anything from the marklin and expecting commands is 6
        if (queue_cur_size == 0) {
          read_many_s88(S88_NOS); 
        }
      }
      // execute from the queue
      
      // print the lft and right pointer of the queue
      uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + COMMAND_ROW + 3, 2);
      uart_printf(CONSOLE,"\033[K");
      uart_printf(CONSOLE, "queue: %u %u queue_cur_size: %u", execution_queue_begin, execution_queue_end, queue_cur_size);
      print_sw_states(TOP_ROW + SW_ROW, LEFT_COL + 1);
      // set coulor to white
      uart_puts(CONSOLE,"\033[37m");
      print_updated_sensors(TOP_ROW + SENSORS_ROW, LEFT_COL + THIRD_COL + 1);
      
      execution_time = get_timerLO();
    }
    // Now no eed at least 10 god danm cycles to complete this shit
    // TOTALLY UNACCEPTABLE
    // The while loop would busy wait for all element in the queue. If there exist nothing in the queue then
    // the program moves on. 
    uint8_t trys = 0; 
    
    while(uart_getc_queue(MARKLIN) && 1 <= expecting_commands && expecting_commands <= S88_NOS){
      // show_timer(get_timerHI(), get_timerLO()); 
      trys++;
      read_marklin(expecting_commands, expecting_byte); 
      update_the_triggered_sensors(expecting_commands, expecting_byte);
      // the expecting byte can by 0 or 1
      // before incrementing if it is equal to 1 then we need to overflow to the expecting_commands
      // uart_printf(CONSOLE,"\033[%u;%uHexpecting_commands: %u expecting_byte: %u ",TOP_ROW + COMMAND_ROW + 2, LEFT_COL + 1, expecting_commands, expecting_byte);
      expecting_byte++;
      if (expecting_byte == 2){
        print_marklin(TOP_ROW + MARKLIN_ROW, LEFT_COL + SECOND_COL + 1, expecting_commands);
        print_activated(TOP_ROW + ACTIVATED_SWITCHES_ROW, LEFT_COL + SECOND_COL + 1, expecting_commands);
        
        expecting_commands++;
        expecting_byte = 0;
      }
    }
    if (expecting_commands > S88_NOS){
      expecting_commands = 0;
      expecting_byte = 0;
    }

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
            print_error(" ");
            // the command changes
            // this would keep printing the command even though the command does not change`
          } else {
            print_error("ERROR:COMMAND LINE EXCEEDED MAX LENGTH");
          }
        }else{

        }

    }
    
    // print the time it takes the loop to finnish at the bottom of the window
    uart_printf(CONSOLE,"\033[%u;%uH",TOP_ROW + WINDOW_HEIGHT - 1, LEFT_COL + 1);
    //uart_printf(CONSOLE,"\033[K");
    uart_printf(CONSOLE, "loop time: %u", get_timerLO() - loop_time);
    if (max_loop_time < get_timerLO() - loop_time){
      max_loop_time = get_timerLO() - loop_time;
      uart_printf(CONSOLE, " max time: %u", max_loop_time);
    }
    
    // uart_printf(CONSOLE,"\033[2J");
  }
  uart_puts(CONSOLE, "\r\n");
  
  // U-Boot displays the return value from main - might be handy for debugging
  return 0;
}