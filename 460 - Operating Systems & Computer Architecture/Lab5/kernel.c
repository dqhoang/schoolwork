/***********************************************************
    kfork creates a child process ready to run in Kmode from body()
                  In addition:
(1) it loads /bin/u1 file to the child's Umode segment 
(2). initialize new PROC's ustack per notes #5.1
************************************************************/
int kfork(char *filename)
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

    p->sp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
    
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

int do_exec( int in )
{
  char s[64];
  int i;
  
  u16 segment = (running->pid + 1 ) * 0x1000;
  s[0] = 'b';
  s[1] = 'i';
  s[2] = 'n';
  s[3] = '/';
  for (i=4;i< 64 ;++i)
  {
    s[i] = get_byte(segment, in + (i-4) );
    if ( s[i] == '\0')
      break;
  }
  running->uss = segment;
  running->usp = segment - 24;
  load( s , segment);
  reset_segment(running->uss, running->usp);
  
  return 1;
}

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
