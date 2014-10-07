#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

// Exports to Assembly
int procSize = sizeof(PROC);

int nproc = 0;

// Imports from assembly
int body();
int goUmode();
int color;
int int80h(), tinth(), kbinth();
int sinth();
int pinth();

int chcolor();

char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };               
               
OFT  oft[NOFT];
PIPE pipe[NPIPE];

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/******** YOUR .c files up to fork-exec ***********/
#include "kernel.c"
#include "wait.c"
#include "pipe.c"   // YOUR pipe.c file
#include "int.c"
#include "forkexec.c"

#include "vid.c"
#include "timer.c"

#include "semaphore.c"
#include "serial.c"
#include "kbd.c"
#include "printer.c"

extern KBD kbd;
int init()
{
    PROC *p;
    int i, j;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        p->time = 0;
        p->inkmode = Kmode;
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];

        for (j=0; j<NFD; j++)
             p->fd[j] = 0;
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;
   
    for (i=0; i<NOFT; i++)
        oft[i].refCount = 0;
    for (i=0; i<NPIPE; i++)
        pipe[i].busy = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->priority = 0;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int scheduler()
{
    if (running->status == RUNNING){
       running->status = READY;
       enqueue(&readyQueue, running);
    }
    running = dequeue(&readyQueue);
    running->status = RUNNING;
    running->time = 10;
}

int set_vec(u16 vector, u16 addr)
{
    u16 location,cs;
    location = vector << 2;
    cs = 0x1000;
    put_word(addr, 0x0000, location);
    put_word(cs,0x0000,location+2);
}

int printSplash()
{
 printf("  _____ _                     _     ____             _           \n");
 printf(" / ____| |                   | |   |  _ \\           (_)          \n");
 printf("| (___ | | ___   _ _ __   ___| |_  | |_) | ___  __ _ _ _ __  ___ \n");
 printf(" \\___ \\| |/ / | | | '_ \\ / _ \\ __| |  _ < / _ \\/ _` | | '_ \\/ __|\n");
 printf(" ____) |   <| |_| | | | |  __/ |_  | |_) |  __/ (_| | | | | \\__ \\\n");
 printf("|_____/|_|\\_\\\\__, |_| |_|\\___|\\__| |____/ \\___|\\__, |_|_| |_|___/\n");
 printf("              __/ |                             __/ |            \n");
 printf("             |___/                             |___/             \n");
}

int printBar()
{
  int co = color;
  int r = row, c = column;
  
  color = DBLACK;
  row = 24; column = 0;
  printf("                                                                               ");
  row = 24;column = 0;
  printf("Skynet Begins");
  
  row =r; column = c;
  color = co;
}
    
main()
{
    vid_init();
    //printSplash();
    vidClear();
    printBar();
    row = 0; column = 0; 
    init();      // initialize and create P0 as running
    // Kernel interrupts
    set_vec(80,int80h);

    kfork("/bin/u1");     // P0 kfork() P1
       // Keyboard Interupts
    set_vec(9, kbinth); kbd_init();
    
    // Printer Interrupts
    set_vec(15, pinth); pr_init();
    
    // Serial interupts
    set_vec(12, sinth);
    set_vec(11, sinth); sinit();
    
    int_off();//lock();
    // Timer Interupts
    set_vec( 8 , tinth); timer_init();
    
    while(1){
      printf("P0 running\n");
      if (nproc==2 && proc[1].status != READY )
      { 
          printf("no runable process, system halts\n");
      }
      while(!readyQueue);
      printf("P0 switch process\n");
      tswitch();   // P0 switch to run P1
   }
}
