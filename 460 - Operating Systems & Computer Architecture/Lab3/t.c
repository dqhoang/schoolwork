#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };
int color;

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/

#include "wait.c"
#include "kernel.c"
#include "int.c"

int init()
{
  int i=0, j=0;
  PROC *p;
  //color = 0x0B;  // RED for putc()
  //printf("init ....\n");
  for (i=0; i < NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid = 0;
    p->parent = 0;
    p->next = (PROC *)&proc[ (i+1) ];
    strcpy(p->name, pname[i]);
    
  }
  proc[NPROC-1].next = 0;
  freeList = &proc[0];//0;
  readyQueue = 0;
  sleepList = 0;

  p = dequeue(&freeList);//get_proc();
  running = p;
  running->status = READY;
  running->parent = p;
  running->pid = 0;
  running->next = 0;
  running->priority = 0;
  
  nproc = 1; 
  
} 

int int80h();  // tell compiler int80h() is a function

int set_vec(u16 vector, u16 handler)
{
  // Write C code to install handler for vector
  u16 location = vector * 4;
  put_word( handler, 0 , location);
  put_word( 0x1000,0,location + 2);
}
            
main()
{
  //color = 0x02;
  printf("\nSkynet Begins!\n");
  init();      // initialize and create P0 as running
  set_vec(80, int80h);

  kfork("/bin/u1");     // P0 kfork() P1
  while(1){
    while(!readyQueue); // P0 loops if no runnable process
      printf("P0 switch process\n");
    tswitch();         // P0 switch to run P1
  }
}

int scheduler()
{
    if (running->status == READY){
        enqueue(&readyQueue, running);
    }
    running = dequeue(&readyQueue);
}
