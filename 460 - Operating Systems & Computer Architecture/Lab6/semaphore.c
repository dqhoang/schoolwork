typedef struct semaphore{
         //u16 lock;    // for Multiprocessors: ensure one process at a time
         int value;   
         PROC *queue;
}SEMAPHORE;

int P(SEMAPHORE *s)
{
    //int sr = lock(); 

    s->value--;
    if (s->value < 0){
       //printf("P() p[%d] sem[%d]",running->pid, s->value);       
       running->status = BLOCK;
       enqueue(&s->queue, running);
       tswitch();
    }
    //unlock(sr);
}

int V(SEMAPHORE *s)
{
    PROC *p;
    //int sr = lock(); 
    s->value++;
    if (s->value <= 0){
       p = dequeue(&s->queue);
       p->status = READY;
       enqueue(&readyQueue, p);
       //printf("proc %d unblocked in V()\n", p->pid); 
    }
    
    //unlock(sr);
}
