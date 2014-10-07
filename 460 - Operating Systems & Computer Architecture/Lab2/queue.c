int enqueue(PROC **tmp, PROC *p)
{
  PROC *prev = tmp, *queue = *tmp;
  //printf("top[%d]\n", (*tmp)->pid);
  if( queue != 0){
    while( queue != 0 && ( p->priority <= (queue)->priority))
    {
      //printf("p[%d,%d] q[%d,%d] ", p->pid, p->priority,(*queue)->pid, (*queue)->priority);
      prev = queue;
      queue = queue->next;
    }
    p->next = queue;
    prev->next = p;
  }else
  {
    //printf("Start\n");
    p->next = *tmp;
    (*tmp) = p;
  }
}

PROC *dequeue(PROC **queue)
{
  PROC *p = (*queue);
  if(*queue != 0)
  {
    *queue = (*queue)->next;
  }else{
    printf("\tDQ - EMPTY ");
  }
  return p;
}

PROC *deSleep( PROC *proc)
{
  PROC *p, *curr = sleepList->next, *prev = sleepList;
  // proc is top
  if (prev == proc)
  {
    sleepList = curr;
    return prev;
  }
  curr = curr->next;
  while(curr>0)
  {
    printf("Tmp[%d] Proc[%d]\n",curr->pid,proc->pid);
    if (curr->pid==proc->pid)
    {
      prev->next = curr->next;
      p = dequeue(&curr);
      //printf("popping {%d}\n",p->pid);
      //printQueue(sleepList);
      return p;
    }
    prev = curr;
    curr = curr->next;
  }
  return -1;
}

PROC *get_proc()
{
  //printf("freeL -");
  return (dequeue (&freeList));
}

int put_proc(PROC *p)
{
  enqueue (&freeList, p);
}
    
printQueue(PROC *queue)
{
//print the queue entries in [pid, priority]->  format;
  PROC *tmp = queue;
  while( tmp != 0 )
  {
    printf("[%d, %d] => ", tmp->pid, tmp->priority);
    if (tmp->next == -1 ){break;}
    tmp = tmp->next;
  }
  printf("\n");
}