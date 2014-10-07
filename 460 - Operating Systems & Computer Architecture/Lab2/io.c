#include <stdarg.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define BASE 10

char *tab = "0123456789ABCDEF";

int prints(char *s)
{
  while(*s)
    putc(*s++);
}

int gets(char *s)
{
  while( (*s = getc()) != '\r')
  {
    putc(*s++);
  }
  *s = 0;
}

int rpu(u32 x)
{
  char c;
  if (x){
    c = tab[x % BASE];
    rpu(x/BASE);
    putc(c);
  }
}

int printu(u16 x)
{
  if (!x){
    putc('0');
    return;
  }
  rpu( (u32)x);
}

int printl(u32 x)
{
  if (!x){
    putc('0');
    return;
  }
  rpu(x);
}

int printx( int x )
{
  int y;

  y = x % 16;
  if(x - y == 0) {
    putc(tab[y]);
  }
  else {
    printx((x-y)/16);
    putc(tab[y]);
  }
  
}

int printd(int x) {
  if(x < 0) {
    putc('-');
    x = x*(-1);
  }else if (x == 0)
  {
    putc('0');
    return;
  }
  rpu( (u32)x);
}

void printf(char *fmt, ...) {
  char *cp = fmt;
  va_list valist;
  
  va_start(valist, fmt);

  while(*cp) {
    if( *cp == '%') {
      ++cp;
      switch(*cp)
      {
	case 's':
	  prints( va_arg(valist, char*) );
	  break;
	case 'c': 
	  putc( va_arg(valist, char*) );
	  break;
	case 'd': 
	  printd( va_arg(valist, int*));
	  break;
	case 'x': 
	  printx( va_arg(valist, u32*));
	  break;
	case 'l': 
	  printl( va_arg(valist, u32*));
	  break;
	case 'u':
	  printu(va_arg(valist, u16*));
	  break;
	default: putc(va_arg(valist, char*));
      }
    }
    else if( *(cp) == '\n') {
      putc('\r');
      putc('\n');
    }
    
    else {
      putc(*cp);
    }
    cp++;
  }
  
  va_end(valist);
}