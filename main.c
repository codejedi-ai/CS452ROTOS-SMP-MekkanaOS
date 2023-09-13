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
void show_timer(const unsigned int hi, const unsigned int lo){
  (void)hi;
  unsigned int minutes = (lo / (unsigned int)1e6) / 60;
  unsigned int seconds = lo / (unsigned int)1e6;
  unsigned int tenth_of_second = lo / (unsigned int)1e5;


  uart_printf(CONSOLE,"\033[H");
  uart_printf(CONSOLE,"\033[?25l");
  uart_printf(CONSOLE, ":%u:%u:%u", minutes, seconds % 60,tenth_of_second%10);

}
// define a function that takes a char array as a parameter
void parse_char_array(char *arr) {
  unsigned char byte1 = 0; // Binary: 00001010
  unsigned char byte2 = 0;  // Binary: 00000001
  char *ptr; // pointer to traverse the array
  char *num[3]; // array to store the numbers
  int i = 1; // index for the array
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
  }
  // print the command
  uart_puts(CONSOLE, num[0]);
  uart_puts(CONSOLE, " ");
  uart_puts(CONSOLE, num[1]);
  uart_puts(CONSOLE, " ");
  uart_puts(CONSOLE, num[2]);
  
  // execute here
  if (num[0][0] == 's'){
    uart_puts(CONSOLE, " SCOMMAND ISSUED");
    // get train number
    char *train_command_ptr = num[1];
    int train_speed = 0; 
    while (*train_command_ptr != '\0') { // loop until the end of the array
        train_speed = 10 * train_speed;
        train_speed += a2d(*train_command_ptr);
        train_command_ptr++; // move to the next character
    }
    char *train_id_ptr = num[2];
    int train_id = 0; 
    while (*train_id_ptr != '\0') { // loop until the end of the array
        train_id = 10 * train_id;
        train_id += a2d(*train_id_ptr);
        train_id_ptr++; // move to the next character
    }
    byte1 = (train_speed); // Binary: 00001010
    byte2 = (train_id);  // Binary: 00000001
    uart_putc(MARKLIN, byte1);
    uart_putc(MARKLIN, byte2);
    uart_putc(MARKLIN, '\r');
  }
}

int kmain() {
  uart_config_and_enable(CONSOLE, 115200);
  uart_config_and_enable(MARKLIN, MARKLIN_BR);
  // move the cursor to the head
  uart_printf(CONSOLE,"\033[H");
  // clear the screen
  uart_printf(CONSOLE,"\033[2J"); 


  unsigned int counter=1, r = 2, col = 1, command_len = 0;
  uart_printf(CONSOLE,"\033[%u;%uH",r,col);
  char hello[] = "THE LINE IS CUT, NO digit edition!!! a2d(), This is d273liu (" __TIME__ ")\r\nPress 'q' to reboot\r\n";
  uart_puts(CONSOLE, hello);


  r += 1;
  col = 1;
  uart_printf(CONSOLE,"\033[%u;%uH",r,col);
  // initialize both console and marklin uarts
  uart_init();

  // not strictly necessary, since line 1 is configured during boot
  // but we'll configure the line anyways, so we know what state it is in

  

  
  
  //uart_printf(CONSOLE, "PI[%u]> ", counter++);
  char c = ' ';

  char* command = NULL; 

  memset(command, 0, COMMANDMAX_LEN);
  
  while (c != 'q') {
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
      uart_printf(CONSOLE,"\033[%u;%uH",r + counter,col);
      uart_printf(CONSOLE, "PI[%u]> ", counter++);
      // uart_puts(CONSOLE, command);
      parse_char_array(command);
      uart_printf(CONSOLE, "\r\n");
      command_len = 0;
      command[command_len] = 0;
    } else if (c == '\b' && command_len > 0){
      command[command_len] = 0;
      command_len--;
    } else {
      
        // this part we need to increase the string length. 
        if (c != 0 ){
          if (command_len + 1 < COMMANDMAX_LEN - 1){
            command[command_len] = c;
            command_len++;
            command[command_len] = 0;
            // the command changes
            // this would keep printing the command even though the command does not change`
            uart_printf(CONSOLE,"\033[%u;%uH",r + counter,col);
            uart_printf(CONSOLE,"\033[K");  
            uart_puts(CONSOLE, command);
          }
          else{
            uart_puts(CONSOLE, "ERROR: COMMAND LENGTH EXCEEDED\r\n"); 
            r++; 
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
