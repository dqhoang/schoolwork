//*************************************************************************
//                      Logic of init.c 
// NOTE: this init.c creates only ONE login process on console=/dev/tty0
// YOUR init.c must also create login processes on serial ports /dev/ttyS0
// and /dev/ttyS1.. 
//************************************************************************

int pid, child, status;
int stdin, stdout;

#include "ucode.c"

main(int argc, char *argv[])
{
//  1. open /dev/tty0 as 0 (READ) and 1 (WRTIE) in order to display messages
  stdin = open("/dev/tty0", 0);
  stdout = open("/dev/tty0", 1);
//  2. // Now we can use printf, which calls putc(), which writes to stdout
  printf("SkyInit : fork a login task on console\n"); 
  child = fork();

  if (child)
    parent();
  else             // login task
    login();
}       

int login()
{
  exec("login /dev/tty0");
}
      
int parent()
{
  int ch1;
  while(1){
    printf("SkyInit : waiting .....\n");

    pid = wait(&status);

    if (pid == child)
    {
      ch1 = fork();
      if( !ch1)
      {
        login();
      }
    }else
    {   
      printf("SkyInit : buried an orphan child %d\n", pid);
    }
  }
}
