#include "ucode.c"

int main(int argc, char *argv[])
{ 
  char name[64]; int pid, cmd, segment, i;
  //syscall(93,0); 
  printSplash();
  printf("enter main() : argc = %d\n", argc);
  for (i=0; i<argc; i++)
       printf("argv[%d] = %s\n", i, argv[i]);
      
  while(1){
    pid = getpid();
    
    for(i = 0; i < 64; i++)
    {
      name[i] = 0;
    }
    segment = (pid+1)*0x1000;   
    
    //printf("==============================================\n");
    printf(" __                                                     __ \n");
    printf("| _| Proc[%c]     *******Menu******       Seg[%x]  |_ |\n",pid+'0',segment);
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
	continue;

    cmd = find_cmd(name);

    switch(cmd){
      case 0 : getpid();   break;
      case 1 : ps();       break;
      case 2 : chname();   break;
      case 3 : kmode();    break;
      case 4 : kswitch();  break;
      case 5 : wait();     break;
      case 6 : exit();     break;
      case 7 : fork();     break;
      case 8 : exec();     break;

      case 9:  pipe();     break;
      case 10: pfd();      break;
      case 11: read_pipe();  break;
      case 12: write_pipe(); break;
      case 13: close_pipe(); break;
      
      case 14: chcolor();	break;
      case 15: sleep();     break;
      
      case 16: sget();      break;
      case 17: sput();      break;
      
      case 18: print();     break;

      default: invalid(name); break;
    } 
  }
}


