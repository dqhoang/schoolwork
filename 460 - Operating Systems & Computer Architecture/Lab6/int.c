/************** syscall routing function *************/
#define PA 13
#define PB 14
#define PC 15
#define PD 16
#define AX  8
 
int kcinth()
{
   u16    segment, offset;
   int    a,b,c,d, r, co;

   segment = running->uss; 
   offset = running->usp;

   a = get_word(segment, offset + 2*PA);
   b = get_word(segment, offset + 2*PB);
   c = get_word(segment, offset + 2*PC);
   d = get_word(segment, offset + 2*PD);

   //co = color;
   //color = 0x0f;
   switch(a){
	case 0 : r = running->pid;     break;
	case 1 : r = do_ps();          break;
	case 2 : r = chname(b);        break;
	case 3 : r = kmode();          break;
	case 4 : r = tswitch();        break;
	case 5 : r = do_wait(b);       break;
	case 6 : r = do_exit(b);       break;
        
	case 7 : r = fork();           break;
	case 8 : r = exec(b);          break;
	
  case 9 : r = chcolor(b);	   break;
  case 10: r = pause(b);       break;

  case 20: r = ugets(b,c);    break;
  case 21: r = uputs(b,c);    break;
  
  case 25: r = prline(b);     break;

   /****** these are YOUR pipe functions ************/
	case 30 : r = kpipe(b); 	   break;
	case 31 : r = read_pipe(b,c,d);  break;
	case 32 : r = write_pipe(b,c,d); break;
	case 33 : r = close_pipe(b);     break;
	case 34 : r = pfd();             break;
  /**************** end of pipe functions ***********/

	case 90: r =  getc();          break;
	case 91: r =  putc(b);         break;  
  
  case 93: r =  vidClear();     break;
       
	case 99: do_exit(b);           break;
	default: printf("invalid syscall # : %d\n", a); 
   }
   put_word(r, segment, offset + 2*AX);
   //color = co;
}

int getppid()
{
  return running->ppid;  
}

int do_ps()
{
  int i;
  PROC *p;
  printf("PID PPID Status    Name\n");
  for (i=0;i<NPROC;++i)
  {
    p = &proc[i];
    printf("%d  %d   ",p->pid,p->ppid);
    switch(p->status)
    {
      case FREE:    printf("FREE      ");               break;
      case READY:   printf("READY     ");               break;
      case RUNNING: printf("RUNNING   ");		break;
      case BLOCK: printf("STOPPED   ");		break;
      case SLEEP:   printf("SLEEP[%d] ", p->event);     break;
      case ZOMBIE:  printf("ZOMBIE    ");               break;
    }
    printf("%s\n", p->name);
  }
  printList("freelist  ", freeList);
  printList("readyQueue", readyQueue);
  printList("sleepList ", sleepList);
  return 0;
}


int kmode()
{
    body();
}

int exit(int v)
{
    return kexit(v);
}

int chname( char* b)
{
  int i=0;
  printf("new name:");
  for(;i<32; ++i)
  {
    running->name[i] = get_byte(running->uss,b+i);
  }
  if (running->name[31] != '\0')
    running->name[31] == '\0';
  printf("%s\n",running->name);
}

int wait(int b)
{
   int pid, status = 0;
   pid = kwait(&status);
   put_word( status , running->uss, b);
   printf("comming out of wait [%d] b:[%d]\n", status, b);
   return pid;
}
