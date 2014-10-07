int ksleep(int event)
{
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
  while(tmp > 0)
  {
    if (tmp->event != event) // Nope Increment
    {
      p=tmp;
      tmp = tmp->next;
      continue;
    }
    //tmp->event = 0;
    tmp->status = READY;		// make it READY
    enqueue(&readyQueue, tmp);	// enter p into readyQueue (by pri)
    // remove p from sleepList if you implement a sleepList  
    if ( tmp == sleepList )	// front of sleepList
    {
      sleepList = tmp->next;
      tmp = p = sleepList;
    }else			// Skip 
    {
      p->next = tmp->next;
      tmp = p->next;
    }
    
    if(tmp == -1){break;}
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
  put_proc(running);
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
  /*
  (1). if no child return -1
  (2). if (can find a ZOMBIE child){
          collect ZOMBIE's pid; 
          write ZOMBIE's exitCode to *status;
          put ZOMBIE PROC into freeList as FREE
          return ZOMBIE's pid;
       }
  (3). ksleep(& of own PROC);
       goto (2);
  */
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
	return i;
      }
    }
    ksleep(running->pid);
  }
}