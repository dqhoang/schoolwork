// P,V on semaphores for process synchronization
// int_off()/int_on() are defined in assembly file.

int P(SEMAPHORE *s)
{
  int sr = int_off(); 

    s->value--;
    if (s->value < 0){
       printf("proc %d blocked in P() at semaphore = %x\n",unning->pid, s);
       running->status = BLOCK;
       enqueue(&s->queue, running);
       tswitch();
    }

    int_on(sr);
}

int V(SEMAPHORE *s)
{
   PROC *p;
   int sr = int_off();
   
   s->value++;
   if(s->value <= 0)
   {
     p = dequeue(&s->queue);
     p->status = READY;
     enqueue(&readyQueue, p);
     printf("proc %d unblocked in V()\n",p->pid);
   }
}

/********  bgetc()/bputc() by polling *********/
int bputc(int port, int c)
{
    while ((in_byte(port+LSR) & 0x20) == 0);
    out_byte(port+DATA, c);
}

int bgetc(int port)
{
    while ((in_byte(port+LSR) & 0x01) == 0);
    return (in_byte(port+DATA) & 0x7F);
}

int enable_irq(unsigned irq_nr)
{
   out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}
   
/************ serial ports initialization ***************/
int sinit()
{
  int i;  
  struct stty *t;
  char *q; 

  /* initialize stty[] and serial ports */
  for (i = 0; i < NR_STTY; i++){
    q = p;

    prints("sinit:"); printi(i);

      t = &stty[i];

      /* initialize data structures and pointers */
      if (i==0)
          t->port = 0x3F8;    /* COM1 base address */
      else
          t->port = 0x2F8;    /* COM2 base address */  

      t->inchars.value = t->inlines.value = 0;
      t->inlines.queue = t->inchars.queue = 0;
      t->mutex.value  = 1;
      t->mutex.queue = 0;
      t->outspace.value = OUTBUFLEN;
      t->outspace.queue = 0;
      t->inhead = t->intail = 0;
      t->ehead =  t->etail = t->e_count = 0;
      t->outhead =t->outtail = t->tx_on = 0;

      // initialize control chars; NOT used in MTX but show how anyway
      t->ison = t->echo = 1;   /* is on and echoing */
      t->erase = '\b';
      t->kill  = '@';
      t->intr  = (char)0177;  /* del */
      t->quit  = (char)034;   /* control-C */
      t->x_on  = (char)021;   /* control-Q */
      t->x_off = (char)023;   /* control-S */
      t->eof   = (char)004;   /* control-D */

    lock();  // CLI; no interrupts

      out_byte(t->port+MCR,  0x09);  /* IRQ4 on, DTR on */ 
      out_byte(t->port+IER,  0x00);  /* disable serial port interrupts */

      out_byte(t->port+LCR,  0x80);  /* ready to use 3f9,3f8 as divisor */
      out_byte(t->port+DIVH, 0x00);
      out_byte(t->port+DIVL, 12);    /* divisor = 12 ===> 9600 bauds */

      /******** term 9600 /dev/ttyS0: 8 bits/char, no parity *************/ 
      out_byte(t->port+LCR, 0x03); 

      /*******************************************************************
        Writing to 3fc ModemControl tells modem : DTR, then RTS ==>
        let modem respond as a DCE.  Here we must let the (crossed)
        cable tell the TVI terminal that the "DCE" has DSR and CTS.  
        So we turn the port's DTR and RTS on.
      ********************************************************************/
      out_byte(t->port+MCR, 0x0B);  /* 1011 ==> IRQ4, RTS, DTR on   */
      out_byte(t->port+IER, 0x01);  /* Enable Rx interrupt, Tx off */

    unlock();
    
    enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    /* show greeting message */
    //USE bputc() to PRINT MESSAGE ON THE SERIAL PORT: serial port # ready
  }
}  
         
//======================== LOWER-HALF ROUTINES ===============================

int shandler(int port)
{  
   struct stty *t;
   int IntID, LineStatus, ModemStatus, intType, c;

   t = &stty[port];            /* IRQ 4 interrupt : COM1 = stty[0] */

   IntID     = in_byte(t->port+IIR);       /* read InterruptID Reg */
   LineStatus= in_byte(t->port+LSR);       /* read LineStatus  Reg */    
   ModemStatus=in_byte(t->port+MSR);       /* read ModemStatus Reg */

   intType = IntID & 7;     /* mask out all except the lowest 3 bits */
   switch(intType){
      case 6 : do_errors(t);  break;   /* 110 = errors */
      case 4 : do_rx(t);      break;   /* 100 = rx interrupt */
      case 2 : do_tx(t);      break;   /* 010 = tx interrupt */
      case 0 : do_modem(t);   break;   /* 000 = modem interrupt */
   }
   out_byte(INT_CTL, ENABLE);   /* reenable the 8259 controller */ 
}

int do_errors()
{ printf("ignore error\n"); }

int do_modem()
{  printf("don't have a modem\n"); }


/* The following show how to enable and disable Tx interrupts */

en_tx(struct stty *t)
{
  // write code to turn on TX interrupt
}

disable_tx(struct stty *t)
{ 
  // write code to turn off TX interrupt
}

/******** echo char to serial port **********/
int secho(struct stty *t, int c)
{
   /* insert c into ebuf[]; turn on tx interrupt */
}

int do_rx(struct stty *t)
{ 
  int c;
  c = in_byte(tty->port) & 0x7F;  /* read the char from port */
  printf("\nrx interrupt c="); putc(c); 

  // COMPLETE with YOUR C code
}      

/*********************************************************************/

int do_tx(struct stty *t)
{
  int c;
  // if ebuf AND outbuf are both empty ==>
  //    turn off tx interrupt; return

  if (ebuf not empty) { 
     // out a char from ebuf; return;
  } 

  if (outbuf not empty){
     // out a char from outbuf; 
     // V any process blocked on outbuf room
  }
}
     
int sgetc(int port)  // port = 0 or 1 
{ 
    // map port # to stty sturcutre
    // BLOCK if no input char yet

    lock();    
      get a char c from inbuf[ ]
    unlock();
    return(c);
}

int sputc(char c, int port)
{
    // map port # to stty pointer
    // BLOCK if outbuf has no room
    lock();              
      enter c into outbuf[ ];
    unlock();
    enable tx interrupt if it is off;    
}

int sgetline(int port, char *line)
{
  // WRITE C code to get a line from a serial port
}

int sputline(int port, char *line)
{
  // WRITE C code to output a line to a serial port
}

int usgets(int port, char *y)
{  
  // get a line from serial port and write line to y in U space
}

int uputs(int port, char *y)
{
  // output the line y in U space to serail port
}