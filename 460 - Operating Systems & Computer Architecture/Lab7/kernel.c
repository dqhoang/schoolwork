int reset_segment(u16 segment, u16 usp)
{
  int j;

  put_word(segment, segment, usp);      //uDS
  put_word(segment, segment, usp+2);    //uES

  for(j = 4; j < 20; j+=2){
    put_word(0, segment, usp+j);
  }

  put_word(segment, segment, usp+20);   //uCS
  put_word(0x0200, segment, usp+22);    //flag
}

PROC* kfork(char *filename)
{
  int j;
  u16 segment;
  PROC *p = get_proc(&freeList);
  if (p)
  {
    p->status = READY;
    p->ppid = running->pid;
    p->priority = 1;
    p->next = 0;
    p->parent = running;
    
    // Initialize Stack
    for (j=1; j<10; ++j)
      p->kstack[SSIZE - j] = 0;          // all saved registers = 0
    
    //p->kstack[SSIZE-1]=(int)goUmode;

    p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
    p->inkmode = 1;
    p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
    
    // put in readyqueue
    enqueue( &readyQueue, p);
    ++nproc;
    
    // Initialize ustack
    // Proc's data sits in this segment
    segment = ( p->pid +1 ) * 0x1000;	// proc 0 sits on 0x1000, proc 1 on 0x2000... 
    // record where we set this guy up at
    p->uss = segment;
    // 12 registers each 2 bytes
    p->usp = segment - 24;
    if (filename != 0)
    {
      load( filename, segment);
    }
    // for every 2 byte register put the right value
    reset_segment(p->uss,p->usp );
    
    return p;
  }else
  {
    printf("No Mo Procs\n");
    return 0;
  }
}

int do_tswitch()
{
  printf("proc %d tswitch()\n", running->pid);
  tswitch();
  printf("proc %d resumes\n", running->pid);
}

int do_kfork()
{
  PROC *p;
  printf("proc%d kfork a child\n");
  p = kfork("/bin/u1");
  if (p == 0)
    printf("kfork failed\n");
  else
    printf("child pid = %d\n", p->pid);
}

int do_exit(int exitValue)
{
  //int exitValue;
  if (running->pid == 1 && nproc > 2){
      printf("other procs still exist, P1 can't die yet !%c\n",007);
      return -1;
  }
  kexit(exitValue);
}

int do_wait(int *ustatus)
{
  int child, status;
  child = kwait(&status);
  if (child<0){
    printf("proc %d wait error : no child\n", running->pid);
    return -1;
  }
  printf("proc %d found a ZOMBIE child %d exitValue=%d\n", 
	   running->pid, child, status);
  // write status to Umode *ustatus
  put_word(status, running->uss, ustatus);
  return child;
}

int body()
{
  char c;
  printf("proc %d resumes to body()\n", running->pid);

  while(1){
    printf("-----------------------------------------\n");
    printList("freelist  ", freeList);
    printList("readyQueue", readyQueue);
    printList("sleepList ", sleepList);
    printf("-----------------------------------------\n");

    printf("proc %d running: parent = %d  enter a char [s|f|w|q|u] : ", 
	   running->pid, running->parent->pid);
    c = getc(); printf("%c\n", c);
    switch(c){
       case 's' : do_tswitch();   break;
       case 'f' : do_kfork();     break;
       case 'w' : do_wait();      break;
       case 'q' : do_exit();      break;
       case 'u' : goUmode();      break;
    }
  }
}

int kmode()
{
  body();
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
      case STOPPED: printf("STOPPED   ");		break;
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

int chname(char * b)
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
