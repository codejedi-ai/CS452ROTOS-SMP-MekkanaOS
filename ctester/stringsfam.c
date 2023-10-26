# include <stdio.h>
# include <stdint.h>

int strcpy(char *dest, int lenDes, char *src, int lenSrc){
	int i = 0;
	while (*src) {
		if (i >= lenSrc) {break;}
		if (i >= lenDes) {break;}
		*dest = *src;
		dest++;
		src++;
		i++;
	}
	*dest = 0;
	return i;
}

int strcmp_ret(char* s1, char* s2){
	while(*s1 && *s2){
		// // uart_printf(CONSOLE, "Addr: %x, %x compsiring: ", s1, s2);
		// uart_putc(CONSOLE, *s1);
		// uart_putc(CONSOLE, *s2);
		// // uart_printf(CONSOLE, "\r\n");
		if(*s1 != *s2){
            return 0;
		}
		
		s1++;
		s2++;
	}
		// // uart_printf(CONSOLE, "compairing last char: ");
		// uart_putc(CONSOLE, *s1);
		// uart_putc(CONSOLE, *s2);
	return (*s1 == *s2);
}
// this fiunction would
// return strings in place
void parsestring(char * str, int part, char *retarr, int size) {
  int i = 1, j = 0;
  retarr[0] = 0;
  // print size
  printf("size = %d\n", size);
  while (*str) {
    if (*str == ' ') {
      i++;
      retarr[j] = 0;
    }else if (part == i){
      if (j >= size - 1) {
        printf("Error: string too long j = %d size - 1 = %d\n", j, size - 1);
        break;
      }
      retarr[j++] = *str;
      retarr[j] = 0;
      printf("%c", (*str));
    }
    str++;
  }
  
}
void printstr(char *str) {
  while (*str) {
    printf("%c %d\n", (*str),(*str));
    str++;
  }
  printf("\n");
}
int stringconcat(char* dest, const char* src) {
  int newsz = 0;
  while (*dest) {
    dest++;
    newsz++;
  }
  while (*src) {
    *dest = *src;
    dest++;
    src++;
    newsz++;
  }
  *dest = '\0';
  return newsz;
}
struct game
{
	uint32_t tid1;
	uint32_t tid2;
	char tid1_move[10];
	char tid2_move[10];
	uint32_t tid1_score;
	uint32_t tid2_score;
};
uint8_t full_game(struct game *cur_game)
{
	/* data */
	return cur_game->tid1 != 0 && cur_game->tid2 != 0;
}
void reset_game(struct game *cur_game)
{
	/* data */
	cur_game->tid1_move[0] = 0;
	cur_game->tid2_move[0] = 0;

}
int check_game(struct game *cur_game){
	if (strcmp_ret(cur_game->tid1_move, "rock") && strcmp_ret(cur_game->tid2_move, "scissors")){
		reset_game(cur_game);
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "scissors") && strcmp_ret(cur_game->tid2_move, "paper")){
		reset_game(cur_game);
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "paper") && strcmp_ret(cur_game->tid2_move, "rock")){
		reset_game(cur_game);
		cur_game->tid1_score++;
		return 1;
	} else if (strcmp_ret(cur_game->tid1_move, "rock") && strcmp_ret(cur_game->tid2_move, "paper")){
		reset_game(cur_game);
		cur_game->tid2_score++;
		return 2;
	} else if (strcmp_ret(cur_game->tid1_move, "scissors") && strcmp_ret(cur_game->tid2_move, "rock")){
		reset_game(cur_game);
		cur_game->tid2_score++;
		return 2;
	} else if (strcmp_ret(cur_game->tid1_move, "paper") && strcmp_ret(cur_game->tid2_move, "scissors")){
		reset_game(cur_game);
		cur_game->tid2_score++;
		return 2;
	}
	return 0;
}
int main1() {
  printf("Hello world \n");
  char *hw = "Hello world";
  char *parsethingy[10];
  parsestring(hw, 1, parsethingy, 10);
  printf("\n");
  // printstr(parsethingy[0]);
  printstr(parsethingy);
  char **parsethingy2[10];
}
int main2(){
  char str1[100];
  char* str2 = "Hello";
  char* str3 = "World";
  int sz = stringconcat(str1, str2);
  sz = stringconcat(str1, str3);
  printf("sz = %d\n", sz);
  printf("str1 = %s\n", str1);
  printf("str2 = %s\n", str2);
  return 0;
}
int main3(){
  struct game cur_game;
  cur_game.tid1 = 1;
  cur_game.tid2 = 2;
  cur_game.tid1_score = 3;
  cur_game.tid2_score = 4;
  printf("full_game = %d\n", full_game(&cur_game));
  return 0;
}
int main4(){
  struct game cur_game;
  cur_game.tid1 = 1;
  cur_game.tid2 = 2;
  cur_game.tid1_score = 3;
  cur_game.tid2_score = 4;
  // cur_game.tid1_move = "paper";
  // cur_game.tid2_move = "rock";
  strcpy(cur_game.tid1_move, 10, "paper", 10);
  strcpy(cur_game.tid2_move, 10, "rock", 10);
  printf("check_game = %d\n", check_game(&cur_game));
  struct game games[10];
  int msglen = 49;
  for (int i = 0; i < 10; i++)
  {
    games[i].tid1 = 1;
    games[i].tid1_move[0] = 0;
    games[i].tid2 = 1;
    games[i].tid2_move[0] = 0;
  }
  for (int i = 0; i < 10; i ++){
    if (!full_game(&games[i])) continue;
    printf("gameserver: game %d is full\r\n", i);
    int check = check_game(&games[i]);
    printf("check = %d\r\n", check);
    /*
    if(check == 2){
      Reply(games[i].tid1, "L", 2);
      Reply(games[i].tid2, "W", 2);
      continue;
    } else if (check == 1){
      Reply(games[i].tid1, "W", 2);
      Reply(games[i].tid2, "L", 2);
      continue;
    }
    */
  }

}
void foo (void* char_ptr, void *x) {
  char *char_ptr_y = (char *) char_ptr; // cast x to char pointer
  int y = * (int *) x; // cast x to int pointer and dereference it
  printf("char_ptr = %s\n", char_ptr_y);
  printf("y = %d\n", y);
}

  
int main5() 
{ 
    void (*array [3]) (void*, void *) = {(void (*) (void *, void *)) foo, 
                                  (void (*) (void *, void *)) foo, 
                                  (void (*) (void *, void *)) foo}; // array of 3 void function pointers

    int a = 1;
    int b = 2;
    int c = 3;
    char *str = "Hello world";
    char *str2 = "Hello world2";
    char *str3 = "Hello world3";
    array [0] (str, &a); // equivalent to foo (&a)
    array [1] (str2, &b); // equivalent to foo (&b)
    array [2] (str3, &c); // equivalent to foo (&c)

    return 0; 
} 
#include <stdarg.h>
/*
void printstringoflenn
takes a string as a parameter. pass teh array whole as the parameter not the pointer
it is a function that takes arbetrary number of params
*/
void printstringoflenn(int len, ...){
  va_list valist;
  va_start(valist, len);
  for (int i = 0; i < len; i++){
    char *str = va_arg(valist, char); // this is just popping the list
    printf("%c\n", str);
  }
  va_end(valist);
}
// a int64 can store 8 chars
// generate a method that takes in a long array and prints 8 chars
void long_to_OctoChars(int64_t charint){
  char *str = (char *) &charint;
  for (int i = 0; i < 8; i++){
    printf("%c\n", str[i]);
  }
}
// an octochar is a char array of length 8 that is encoded as a int64
int64_t octoChar_to_int64(char *str){
  int64_t ret = 0;
  for (int i = 7; i >=0; i--){
    ret = ret << 8;
    ret = ret | str[i];
  }
  return ret;
}
// return 8 characters in place from a int64
void int64_to_octochar(int64_t charint, char *str){
  for (int i = 0; i < 8; i++){
    str[i] = charint & 0xFF;
    charint = charint >> 8;
  }
}
// turn byint- two integers into int64
void int64_to_byint(int64_t charint, int *a, int *b){
  *a = charint >> 32;
  *b = charint & 0xFFFFFFFF;
}
// return int64 that encodes two integers
int64_t byint_to_int64(int a, int b){
  int64_t ret = a;
  ret = ret << 32;
  ret = ret | b;
  return ret;
}
int parse_char_arr(char *arr, char **num, int num_size){
  char *ptr; // pointer to traverse the array
  int i = 1; // index for the array
  num[0] = arr;
  ptr = arr; // point to the first element of the array
  while (*ptr != '\0') { // loop until the end of the array
    printf("ptr = %c\n", *ptr);
    
    if (*ptr == ' ') { // check if the character is a space
      *ptr = 0;
      num[i++] = ptr + 1; // store the value in the array
      if (i >= num_size) {return i;}
      // increment the index
    }
    
    ptr++; // move to the next character
  }
  return i;
}

int logo(){
  /*
  // printstringoflenn(4, 'a','b','c','d');
  int64_t charint = octoChar_to_int64("abcdefgh");
  //printf("charint = %lld\n", charint);
  char str[9];
  int64_to_octochar(charint, str);
  for (int i = 0; i < 8; i++){
    printf("%c\n", str[i]);
  }
  str[8] = 0;
  //printf("str = %s\n", str);
  // test parse char array
  
  char arr[100] = "Hello world";
  arr[11] = 0;
  char *num[10];
  int ret = parse_char_arr(arr, num, 10);
  printf("ret = %d\n", ret);
  printf("num[0] = %s\n", num[0]);
  printf("num[1] = %s\n", num[1]);
  printf("num[2] = %s\n", num[2]);
  */
   char *logo = "            ___     ___     ___     ___   __   __   ___     ___   \r\n    o O O  |   \\   /   \\   | _ \\   / __|  \\ \\ / /  / _ \\   / __|  \r\n   o       | |) |  | - |   |   /  | (__    \\ V /  | (_) |  \\__ \\  \r\n  TS__[O]  |___/   |_|_|   |_|_\\   \\___|   _|_|_   \\___/   |___/  \r\n {======|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_|\"\"\"\"\"|_| \"\"\" |_|\"\"\"\"\"|_|\"\"\"\"\"| \r\n./o--000\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\'\"`-0-0-\' \r\n";
    printf("%s", logo);
  return 0;
}
int main(){
  // encode HELLO HI as a long, put each char as an ascii value in hexadeciman
  // 0x48454c4c4f204849
  // H 
  /*
  uint64_t a = 0x48454c4c4f204849;
  
  char *str = (char *) &a;
  for (int i = 7; i >= 0; i--){
    printf("%c\n", str[i]);
  }
  printf("a = %llx\n", *((uint64_t*)str));
  */
  int channel = 0xffffffff;
 	char channel64[8] = "CTSM";
  /*
  channel64[0] = 0;
  channel64[1] = 1;
  channel64[2] = 2;
  channel64[3] = 3;
  channel64[4] = 4;
  channel64[5] = 5;
  channel64[6] = 6;
  channel64[7] = 7;
  
	// *(uint64_t *)channel64 = 0;
  // char channel64[8];
  *((uint32_t *) channel64 + 1) = ((uint32_t) channel);
  channel64[0] = 0;
  channel64[1] = 0;
  channel64[2] = 0;
  channel64[3] = -1;
	// channel64[2] = 0;
  */  
  uint64_t a = *((uint64_t *)channel64); 
  printf("a = 0x%llx\n", a);
  char *str = (char *)&a;
  for (int i = 0; i < 8; i++){
    printf("0x%x\n", (char)str[i]);
  }
  return 0;
}