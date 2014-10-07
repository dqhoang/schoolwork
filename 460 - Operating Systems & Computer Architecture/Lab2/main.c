/************ t.c file **********************************/
#define NPROC     9        
#define SSIZE  1024                /* kstack int size */

#define FREE      0                /* proc status     */
#define READY     1      
#define	SLEEP     2
#define BLOCK     3
#define ZOMBIE    4	

typedef struct proc{
  struct proc *next;   
  int  ksp;               // saved sp; offset = 2 
  
  int  status;            // FREE|READY|SLEEP|BLOCK|ZOMBIE 
  int  priority;	  // Priority
  int  pid;		  // process pid
  int  ppid;		  // parent pid
  struct proc *parent;	  // pointer to parent PROC
  
  int  kstack[SSIZE];     // kmode stack of task
}PROC;

#include "io.c"  /* <===== use YOUR OWN io.c with printf() ****/

PROC proc[NPROC], *running, *freeList, *readyQueue;

int  procSize = sizeof(PROC);

/****************************************************************
 Initialize the proc's as shown:
        running ---> proc[0] -> proc[1];

        proc[1] to proc[N-1] form a circular list:

        proc[1] --> proc[2] ... --> proc[NPROC-1] -->
          ^                                         |
          |<---------------------------------------<-

        Each proc's kstack contains:
        retPC, ax, bx, cx, dx, bp, si, di, flag;  all 2 bytes
*****************************************************************/

int body();  

int initialize()
{
  int i, j;
  PROC *p;

  for (i=1; i <= NPROC; i++){
    p = &proc[i];
    p->next = &proc[i+1];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    p->ppid =0;
    p->parent = 0;
    
    if (i){     // initialize kstack[ ] of proc[1] to proc[N-1]
      for (j=1; j<10; j++)
          p->kstack[SSIZE - j] = 0;          // all saved registers = 0
      p->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
      p->ksp = &(p->kstack[SSIZE-9]);        // ksp -> kstack top
    }
  }
  running = &proc[0];
  running->status = READY;
  running->parent = &proc[0];
  running->pid = 0;
  running->next = 0;
//  for(i=0;i<8;i++)
//    *freeList[i] = 0;
  proc[NPROC-1].next = &proc[1];
  printf("initialization complete\n"); 
}

char *gasp[NPROC]={
     "Oh! You are killing me .......",
     "Oh! I am dying ...............", 
     "Oh! I am a goner .............", 
     "Bye! Bye! World...............",      
};

int grave(){
  printf("\n*****************************************\n"); 
  printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
  printf("*****************************************\n");
  running->status = FREE;

  tswitch();   /* journey of no return */        
}

int ps()
{
  PROC *p;

  printf("running = %d\n", running->pid);

  p = running;
  p = p->next;
  printf("readyProcs = ");
  while(p != running && p->status==READY){
    printf("%d -> ", p->pid);
    p = p->next;
  }
  printf("\n");
}

int body()
{  char c;
   while(1){
      ps();
      printf("I am Proc(meatbag) %d in life()\n", running->pid);
      printf("Kill Me : [s|q] ");
       c=getc();
       switch(c){
            case 's': tswitch(); break;
            case 'q': grave();   break;
            default :            break;  
       }
   }
}


main()
{
  printf("\nWelcome to Skynet!\n");getc();
  //initialize();
  //printf("P0 switch to P1\n");
  //tswitch();
  //printf("P0 resumes: all dead, happy ending. Yay\n");
}


int scheduler()
{
    PROC *p;
    p = running->next;

    while (p->status != READY && p != running)
      p = p->next;

    if (p == running)
       running = &proc[0];
    else
       running = p;

    printf("\n-----------------------------\n");
    printf("next victim = %d\n", running->pid);
    printf("-----------------------------\n");
}
