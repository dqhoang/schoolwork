
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
int kcinth()
{
  u16 segment = running->uss; 
  u16 offset = running->usp;

  int a,b,c,d, r;
  a=b=c=d=r=0;
  a = get_word(segment, offset + 13*2);
  b = get_word(segment, offset + 14*2);
  c = get_word(segment, offset + 15*2);
  d = get_word(segment, offset + 16*2);
  
  switch(a){
    case 0 : r = running->pid;     break;
    case 1 : r = do_ps();          break;
    case 2 : r = chname(b);        break;
    case 3 : r = kmode();          break;
    case 4 : r = tswitch();        break;
    case 5 : r = wait(b);          break;
    case 6 : r = exit(b);          break;
    case 7 : r = do_fork();	   break;
    case 8 : r = do_exec(b);	   break;
    default: printf("invalid syscall # : %d\n", a); 
  }
  put_word( r, segment, offset + 8*2);
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
      case FREE:    printf("FREE      ");                break;
      case READY:   printf("READY     ");               break;
      case RUNNING: printf("RUNNING   ");		   break;
      case STOPPED: printf("STOPPED   ");		   break;
      case SLEEP:   printf("SLEEP[%d] ", p->event); break;
      case ZOMBIE:  printf("ZOMBIE    ");              break;
    }
    printf("%s\n", p->name);
  }
  printList("freelist  ", freeList);
  printList("readyQueue", readyQueue);
  printList("sleepList ", sleepList);
  return 0;
}

int copy_image( u16 child_segment )
{
  u16 i =0;
  int t = 0;
  while( i < 0x1000 )
  {
    t = get_word( running->uss,i);
    put_word( t , child_segment, i);
    i++;
  }
}

int kmode()
{
    body();
}

int exit(int v)
{
    kexit(v);
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
