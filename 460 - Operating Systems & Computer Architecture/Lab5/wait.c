
int geti()
{
  char s[16];
  gets(s);
  return atoi(s);
}

int ksleep(int event)
{
  printf("proc[%d] going to sleep event[%d]\n", running->pid, event);
  running->status = SLEEP;
  running->event = event;
  // enter sleepList FIFO 
  enqueue(&sleepList, running);
  tswitch();
}

/* wake up ALL procs sleeping on event */
int kwakeup(int event)
{
  // wake up ALL PROCs sleeping on event
  PROC *p, *tmp = sleepList;
  int i;
  printf("Waking up on event [%d]\n", event);
  while(tmp)
  {
    if (tmp->event != event) // Nope Increment
    {
      p=tmp;
      if( tmp->next == 0 )
      {
	return;
      }
      tmp = tmp->next;
      continue;
    }
    tmp->event = 0;
    tmp->status = READY;		// make it READY
    enqueue(&readyQueue, tmp);	// enter p into readyQueue (by pri)
    // remove p from sleepList
    if ( tmp == sleepList )	// front of sleepList
    {
      sleepList = tmp->next;
      tmp = p = sleepList;
    }else			// Skip 
    {
      p->next = tmp->next;
      tmp = p->next;
      if (p->next == 0)
	break;
    }
    if(tmp == 0){break;}
  }
}

int kexit(int exitValue)
{
  int i;
  int *p, parent = running->ppid, wake = 0;
  PROC *tmp = sleepList;
//  (1). send children (dead or alive) to P1's orphanage
  for(i=2;i<9;++i)
  {
    if( proc[i].ppid == running->pid)
    {
      wake = 1;
      proc[i].ppid = 1;
      proc[i].parent = &proc[1];
    }
  }
//  (2). record exitValue in PROC.exitCode and become a ZOMBIE
  running->status = ZOMBIE;
  running->exitCode = exitValue;
  running->next = 0;
  for (i=0; i < NFD ; ++i)
  {
    if( running->fd[i] != 0 )
    {
      close_pipe(i);
    }
  }
  enqueue(&freeList, running );//put_proc(running, &freeList);
//  (3). wakeup parent;
//  (4). wakeup P1 also if has sent any child to P1
  kwakeup(running->ppid);
  if(wake){
    kwakeup(1);
  }
  nproc--;
  tswitch();  
}

int child(int pid)
{
  int i = 2;
  for(;i<NPROC;++i)
  {
    if(proc[i].ppid == pid)
      return i;
  }
  return 0;
}

int kwait(int *status)
{
  int i;
  if (!child(running->pid))
    return -1;
  
  while(1)
  {
    for(i=2;i<NPROC;++i)
    {
      if( proc[i].ppid == running->pid && proc[i].status == ZOMBIE )
      {
	i = proc[i].pid;
	*status = proc[i].exitCode;
	proc[i].status = FREE;
	//enqueue(&freeList, &proc[i] );
	return i;
      }
    }
    ksleep(running->pid);
  }
}