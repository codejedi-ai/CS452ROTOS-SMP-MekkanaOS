#include "rpi.h"
#include "util.h"

// Serial line 1 on the RPi hat is used for the console
static const size_t COMMANDMAX_LEN = 32;
#define UNINT_MAX 0xffffffff
#define OVERFLOW_MINUTES = (UNINT_MAX / 1e6) / 60;
#define OVERFLOW_SECONDS = UNINT_MAX / 1e6;
#define OVERFLOW_TENTH_OF_SECOND = UNINT_MAX / 1e5;
#include <stdio.h>
#include <stdlib.h>
uint32_t sol_on_time= 0;
char sol_is_on = 0;
void show_timer(const unsigned int hi, const unsigned int lo){
  (void)hi;
  unsigned int minutes = (lo / (unsigned int)1e6) / 60;
  unsigned int seconds = lo / (unsigned int)1e6;
  unsigned int tenth_of_second = lo / (unsigned int)1e5;


  uart_printf(CONSOLE,"\033[H");
  uart_printf(CONSOLE,"\033[?25l");
  uart_printf(CONSOLE, "Time:%u:%u:%u", minutes, seconds % 60,tenth_of_second%10);

}
void execute_train_command(unsigned char speed, // Binary: 00001010 
                           unsigned char id){  // Binary: 00000001)
      uart_putc(MARKLIN, speed);
      uart_putc(MARKLIN, id);
      uart_putc(MARKLIN, '\r');
}
void solonoid_command(unsigned char solonoid_id, // Solonoid ID. . 
                    unsigned char direction){  // S 33 go straight, C 34 go bent
      
      if (direction ==  'C')  uart_putc(MARKLIN, 34);
      if (direction ==  'S')  uart_putc(MARKLIN, 33);
      uart_putc(MARKLIN, solonoid_id);
      sol_on_time = get_timerLO();
      sol_is_on = 1;
}
char prev_marklin;
void read_one_s88(char s88_id){  // char datatype is the byte
      uart_putc(MARKLIN, 192 + s88_id);
      uart_putc(MARKLIN, '\r');
}
// read_many_s88 reads in 
void read_many_s88(char s88_no){  // Solonoid ID
      uart_putc(MARKLIN, 128 + s88_no);
      uart_putc(MARKLIN, '\r');
}
// solonoid off
void sol_off(){  // Solonoid ID
      uart_putc(MARKLIN, 32);
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
  // print the command
  uart_puts(CONSOLE, num[0]);
  uart_puts(CONSOLE, " ");
  uart_puts(CONSOLE, num[1]);
  uart_puts(CONSOLE, " ");
  uart_puts(CONSOLE, num[2]);
  
  // execute here
  if (num[0][0] == 't' && num[0][1] == 'r'){
    uart_puts(CONSOLE, " tr command ISSUED");
    // get train number
    char *train_id_ptr = num[1];
    char train_id = 0; 
    while (*train_id_ptr != '\0') { // loop until the end of the array
        train_id = 10 * train_id;
        train_id += a2d(*train_id_ptr);
        train_id_ptr++; // move to the next character
    }

    char *train_command_ptr = num[2];
    char train_speed = 0; 
    while (*train_command_ptr != '\0') { // loop until the end of the array
        train_speed = 10 * train_speed;
        train_speed += a2d(*train_command_ptr);
        train_command_ptr++; // move to the next character
    }

    execute_train_command(train_speed, train_id);
  }else if (num[0][0] == 'r' && num[0][1] == 'v'){
    // This command is just a prototype. It gives the user the function of reversing it at a different speed
    // get train number
    uart_puts(CONSOLE, " rv command ISSUED");
    char *train_command_ptr = num[1];
    char train_speed = 0; 
    while (*train_command_ptr != '\0') { // loop until the end of the array
        train_speed = 10 * train_speed;
        train_speed += a2d(*train_command_ptr);
        train_command_ptr++; // move to the next character
    }
    char *train_id_ptr = num[2];
    char train_id = 0; 
    while (*train_id_ptr != '\0') { // loop until the end of the array
        train_id = 10 * train_id;
        train_id += a2d(*train_id_ptr);
        train_id_ptr++; // move to the next character
    }
    execute_train_command(0,train_id);
    execute_train_command(15,train_id);
    execute_train_command(train_speed,train_id);
  }else if (num[0][0] == 's' && num[0][1] == 'w'){
    uart_puts(CONSOLE, " sw command ISSUED");
    char *switch_number = num[1];
    char sol_id = 0; 
    while (*switch_number != '\0') { // loop until the end of the array
        sol_id = 10 * sol_id;
        sol_id += a2d(*switch_number);
        switch_number++; // move to the next character
    }
    char switch_state = num[2][0];
    solonoid_command(sol_id,  switch_state);
  }
}
void print_typing_command(char *command,int r,int c){
  uart_printf(CONSOLE,"\033[%u;%uH",r,c);
  uart_printf(CONSOLE,"\033[K");  
  uart_puts(CONSOLE, command);
}
int kmain() {
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable(MARKLIN, MARKLIN_BR);
  // move the cursor to the head
  uart_printf(CONSOLE,"\033[H");
  // clear the screen
  uart_printf(CONSOLE,"\033[2J"); 


  unsigned int counter=1, row = 2, col = 1, command_len = 0;
  uart_printf(CONSOLE,"\033[%u;%uH",row,col);
  char hello[] = "ID THEN SPEED: UPDATE, This is d273liu (" __TIME__ ")\r\nPress 'q' to reboot\r\n";
  uart_puts(CONSOLE, hello);


  row += 1;
  col = 1;
  uart_printf(CONSOLE,"\033[%u;%uH",row,col);
  // initialize both console and marklin uarts
  uart_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in
    
    
  // uart_printf(CONSOLE, "PI[%u]> ", counter++);
  char c = ' ';
  char* command = NULL;

  memset(command, 0, COMMANDMAX_LEN);
  
  while (c != 'q') {
    if (sol_is_on > 0 && get_timerLO() - sol_on_time > 200000){
      sol_off();
      sol_is_on = 0;
    }
    // uart_putc(MARKLIN, (unsigned char)32);
    show_timer(get_timerHI(), get_timerLO()); 

    c = uart_getc(CONSOLE);
    uart_printf(CONSOLE,"\033[?25l");
    if (c == '\r') {
      // c is the character the terminal is getting in real time. 
      // I want to print what was displayed. 


      // execute the command 
      // need to parse the command

      
      // need to iterate through the command and get the pointers to the 
      uart_printf(CONSOLE,"\033[%u;%uH",row + counter,col);
      uart_printf(CONSOLE, "PI[%u]> ", counter++);
      // uart_puts(CONSOLE, command);
      parse_char_array(command);
      uart_printf(CONSOLE, "\r\n");
      command_len = 0;
      command[command_len] = 0;
    } else if (c == '\b' && command_len > 0){
      command[--command_len] = 0;
      print_typing_command(command, row + counter,col);
      // I do not know why I need to print this here
    } else {
        // this part we need to increase the string length. 
        if (c != 0 && c != '\b'){
          if (command_len + 1 < COMMANDMAX_LEN - 1){
            command[command_len] = c;
            command[++command_len] = 0;
            print_typing_command(command, row + counter,col);
            // the command changes
            // this would keep printing the command even though the command does not change`
          }
          else{
            uart_puts(CONSOLE, "ERROR: COMMAND LENGTH EXCEEDED\r\n"); 
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
