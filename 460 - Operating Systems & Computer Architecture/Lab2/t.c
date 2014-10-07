/******************* t.c file ****************************/
#define NPROC    9
#define SSIZE 1024
/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

typedef struct proc{
    struct proc *next;
    int    *ksp;
    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
  struct proc *parent;
    int    priority;	       // we go high to low
    int    event;
    int    exitCode;
    int    kstack[SSIZE];      // per proc stack area
}PROC;

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc; // number of active PROCs in system

#include "io.c"
#include "queue.c"
#include "wait.c"
#include "kernel.c"

int init()
{
  int i, j;
  PROC *p; 
  for (i=0; i < NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid = 0;
    p->parent = 0;
    if (i){     // initialize kstack[ ] of proc[1] to proc[N-1]
      enqueue( &freeList, p);
    }
    p->next = (PROC *)&proc[ (i+1) ];
  }
  proc[NPROC-1].next = 0;
  
  running = &proc[0];
  running->status = READY;
  running->parent = &proc[0];
  running->pid = 0;
  running->next = -1;
  
  freeList = &proc[1];
  readyQueue = &proc[0];
  sleepList = 0;
  
  nproc = 1;
} 


int scheduler()
{
  if (running->status == READY)
  {  
    enqueue(&readyQueue, running);
  }
  running = dequeue(&readyQueue);
}
            
main()
{
  printf("\r\n");
  printf("Welcome to Skynet\n");
  
  init();      // initialize and create P0 as running
  kfork();     // P0 kfork() P1
  /*
  while(1){
    printf("P0 running\n");
    while(readyQueue){
      tswitch();   // P0 switch to run P1
    }
  }*/
  if(readyQueue){
    tswitch();
  }
  printf("ALL DEAD, HAPPY ENDING\n");
}