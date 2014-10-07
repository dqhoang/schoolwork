#define BASE 10
#include "io.c"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

char *hextab = "0123456789ABCDEF";

void main() {
  int i = -6;
  u16 u = 5;
  u32 l = 12;
  char cchar = '=';
  int x = 255;
	printf("a string printed.\n");
	printf("%s", "test string\n");
	printf("An int %d \n", i );
	printf("A char %c \n", cchar);
	printf("a u16 %u \n", u);
	printf("a u32 %l \n", l);
	printf("hex %x \n", x);
}
