int body();

PROC *kfork()
{
  int j;
  PROC *p = get_proc();
  //printf("--- FORK ---\n");
  if (p)
  {
    p->status = READY;
    p->ppid = running->pid;
    p->priority = 1;
    p->next = -1;
    p->parent = running;
    
    for (j=1; j<10; ++j)
      p->kstack[SSIZE - j] = 0;          // all saved registers = 0
    p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
    p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
    
    enqueue( &readyQueue, p);
    ++nproc;
    
    return p;
  }else
  {
    printf("No Mo Procs\n");
    return 0;
  }
  
}         

int body()
{
  char c;
  printf("KillModule %d resumes to body()\n", running->pid);
  while(1){
    printf("-----------------------------------------\n");
    printf("Freelist\n");
    printQueue(freeList);
    printf("ReadyQueue\n");
    printQueue(readyQueue);
    printf("Sleeplist\n");
    printQueue(sleepList);
    printf("-----------------------------------------\n");

    printf("KillModule %d[%d] running: parent=%d\n",
	   running->pid, running->priority, running->ppid);

    printf("s:tswitch\nq:exit\nf:fork\nz:sleep\na:wake\nw:wait\n");
    printf("enter a char [s|q|f|z|a|w] : "); // z=sleep a=wakeup w=wait 
    c = getc(); printf("%c\n", c);

    switch(c){
       case 's' : do_tswitch();   break;
       case 'f' : do_kfork();     break;
       case 'q' : do_exit();      break;

       case 'z' : do_sleep();     break;
       case 'a' : do_wakeup();    break;

       case 'w' : do_wait();      break;
    }
  }
}

int geti()
{
  char s[16];
  gets(s);
  return atoi(s);
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
  p = kfork();
  if (p==0)
     printf("kfork failed\n");
  else
     printf("child pid = %d\n", p->pid);
}

int do_sleep()
{
  int event;
  printf("input a value to sleep on : ");
  event = geti();printf("\n");
  printf("KillModule %d going to sleep on %d\n", running->pid, event);
  ksleep(event);
  printf("KillModule %d resume after sleep\n");
}

int do_wakeup()
{
  int event;
  printf("enter a event value to wakeup : ");
  event = geti();
  kwakeup(event);
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