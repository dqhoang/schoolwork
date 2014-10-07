/***********************************************************
    kfork creates a child process ready to run in Kmode from body()
                  In addition:
(1) it loads /bin/u1 file to the child's Umode segment 
(2). initialize new PROC's ustack per notes #5.1
************************************************************/
int kfork(char *filename)
{
  int j;
  u16 size, segment, info;
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
    p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
    p->sp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
    
    // put in readyqueue
    enqueue( &readyQueue, p);
    ++nproc;
    
    // Initialize ustack
    // Proc's data sits in this segment
    size = 0x1000;		// size of one proc's space
    segment = ( p->pid +1 ) * 0x1000;	// proc 0 sits on 0x1000, proc 1 on 0x2000... 

    load( filename, segment);
    // for every 2 byte register put the right value
    for ( j=0; j<13;++j)
    {
      info = 0;
      if(j==0)
      {
	info = 0x0200;
      }
      if (j== 2 || j ==11 || j == 12)
      {
	info = segment;
      }
      put_word(info, segment, size - j*2);
    }
    // record where we set this guy up at
    p->uss = segment;
    // 12 registers each 2 bytes
    p->usp = size - 12*2;
    return p;
  }else
  {
    printf("No Mo Procs\n");
    return 0;
  }
}

int do_tswitch()
{
  printf("KillModule %d tswitch()\n", running->pid);
  tswitch();
  printf("KillModule %d resumes\n", running->pid);
}

int do_kfork()
{
  PROC *p;
  printf("KillModule%d kfork a child\n", running->pid);
  p = kfork("/bin/u1");
  if (p==0)
     printf("kfork failed\n");
  else
     printf("child pid = %d\n", p->pid);
}

int do_exit()
{
  int exitValue;
  if (running->pid == 1 && nproc > 2){
      printf("other killMods still exist, P1 can't die yet !%c\n",007);
      return -1;
  }
  printf("enter an exitValue : ");
  exitValue = geti();
  printf("\n");
  kexit(exitValue);
}

int do_wait()
{
  int child, status;
  child = kwait(&status);
  if (child<0){
    printf("KillModule %d wait error : no child\n", running->pid);
    return -1;
  }
  printf("KillModule %d found a ZOMBIE child %d exitValue=%d\n", 
	   running->pid, child, status);
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
    printList("sleepList ", sleepList); // if you use a sleepList
    printf("-----------------------------------------\n");

    printf("proc %d running: parent = %d  enter a char [s|q|f|w| u ] : ", 
	   running->pid, running->parent->pid);
    c = getc(); printf("%c\n", c);
    switch(c){
       case 's' : do_tswitch();   break;
       case 'f' : do_kfork();     break;
       case 'q' : do_exit();      break;
       case 'w' : do_wait();      break;

       case 'u' : goUmode();      break;  // add this to allow a PROC go Umode
    }
  }
}
