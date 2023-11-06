#include "switchworker.h"

void print_sw_states(char *sw_states, uint32_t r, uint32_t c, uint8_t sw_ind){
  uart_printf(CONSOLE,"\033[%u;%uH",r,c);
  uart_puts(CONSOLE, "SW");
  uint8_t middle_Sw[] ={0x99,0x9a,0x9b,0x9c};
  for (uint32_t i = 0; i < 4; i ++){
    if (sw_ind == middle_Sw[i]){
      uart_printf(CONSOLE,"\033[%u;%uH",r + i + SWITCH_COUNT + 1, c);
      uart_printf(CONSOLE,"T%x: ",  middle_Sw[i]);
      uart_putc(CONSOLE, sw_states[middle_Sw[i]]);
      uart_puts(CONSOLE, "\r\n");
      return; 
    } 
  }
  uart_printf(CONSOLE,"\033[%u;%uH",r + sw_ind, c);
  uart_printf(CONSOLE,"T%u: ", sw_ind);
  uart_putc(CONSOLE, sw_states[sw_ind]);
  uart_puts(CONSOLE, "\r\n");
}
void command_wrapper(int io_TXIC_MARKLIN_server_pid, unsigned char byte_1, unsigned char byte_2 ){
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_1);
    Putc(io_TXIC_MARKLIN_server_pid, MARKLIN, byte_2);
}
void solonoid_command(int io_TXIC_MARKLIN_server_pid, char* sw_states, unsigned char solonoid_id, // Solonoid ID. . 
                      unsigned char direction){  // S 33 go straight, C 34 go bent
    char byte_1 = 0;
    char byte_2 = 0;
    sw_states[solonoid_id] = direction;
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
void switch_worker(){
    RegisterAs("switch_worker");
    int tid;
    int ret;
    char sw_states[SWITCH_MAX_count];
    int io_TXIC_MARKLIN_server_pid = -1;
    while (io_TXIC_MARKLIN_server_pid == -1){
        io_TXIC_MARKLIN_server_pid = WhoIs("io_TXIC_MARKLIN_server");
    }
    uart_printf(CONSOLE, "switch_worker is active\r\n");
    
    while (1)
    {
        char recv_msg[4];
        Receive(&tid, recv_msg, 4);
        uint8_t sw_ind = recv_msg[0];
        char state = recv_msg[1];
        if (state == 0){
            // this is a function call
            // return the state of the switch
            char send_msg[4];
            send_msg[0] = sw_ind;
            send_msg[1] = sw_states[sw_ind];
            Reply(tid, send_msg, 4);
        }else{
            if(state != 'S' && state != 'C'){
                char send_msg[4];
                send_msg[1] = -1;
                print_error("ERROR: INVALID DIRECTION");
                Reply(tid, send_msg, 4);
                continue;
            }
            // this is a set switch
            sw_states[sw_ind] = state;
            char switch_state = sw_states[sw_ind];
            uint8_t sol_id = sw_ind;
            solonoid_command(io_TXIC_MARKLIN_server_pid, sw_states,  sw_ind, state);
            
            // send the message to the switch server
            char send_msg[4];
            send_msg[0] = sw_ind;
            send_msg[1] = state;
            Send(io_TXIC_MARKLIN_server_pid, send_msg, 4, &ret, 4);
            Reply(tid, 0, 0);
        }
    }
    
    Exit();
}

void set_switch(int sw_server_tid, uint8_t sw_ind, char state){
    int ret;
    // send the message to the switch server
    char send_msg[4];
    send_msg[0] = sw_ind;
    send_msg[1] = state;
    Send(sw_server_tid, send_msg, 4, &ret, 4);
}
char get_switch(int sw_server_tid, uint8_t sw_ind){
    int ret;
    // send the message to the switch server
    char send_msg[4];
    send_msg[0] = sw_ind;
    send_msg[1] = 0; // this is function call it 
    Send(sw_server_tid, send_msg, 4, send_msg, 4);
    return send_msg[1];
}