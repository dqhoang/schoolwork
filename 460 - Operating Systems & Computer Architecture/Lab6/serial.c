#define INT_CTL     0x20
#define ENABLE      0x20

#define NULLCHAR      0
#define BEEP          7
#define BACKSPACE     8
#define ESC          27
#define SPACE        32

#define INBUFLEN     64
#define OUTBUFLEN    64
#define EBUFLEN      10
#define LSIZE        64

#define NR_STTY       2    /* number of serial ports */

/* offset from serial ports base */
#define DATA         0   /* Data reg for Rx, Tx   */
#define DIVL         0   /* When used as divisor  */
#define DIVH         1   /* to generate baud rate */
#define IER          1   /* Interrupt Enable reg  */
#define IIR          2   /* Interrupt ID rer      */
#define LCR          3   /* Line Control reg      */
#define MCR          4   /* Modem Control reg     */
#define LSR          5   /* Line Status reg       */
#define MSR          6   /* Modem Status reg      */

/**** The serial terminal data structure ****/
struct stty {
   /* input buffer */
   char inbuf[INBUFLEN];
   int inhead, intail;
   SEMAPHORE inchars;

   /* output buffer */
   char outbuf[OUTBUFLEN];
   int outhead, outtail;
   SEMAPHORE outspace;
   int tx_on;
   
   /* echo buffer */
   char ebuf[EBUFLEN];
   int ehead, etail, e_count;

   /* Control section */
   char echo;   /* echo inputs */
   char ison;   /* on or off */
   char erase, kill, intr, quit, x_on, x_off, eof;
   
   /* I/O port base address */
   int port;
} stty[NR_STTY];

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

/************ serial ports initialization ***************/

int sinit()
{
  int i=0;  
  struct stty *t;
  char *q, *p; 
  
  char *first = "\n\rSerial Port ";
  char *second= " Ready\n\r\007";

  /* initialize stty[] and serial ports */
  for (i = 0; i < NR_STTY; i++){
    q = first; p = second;
    prints("sinit:"); printf("%d\n",i);

      t = &stty[i];

      /* initialize data structures and pointers */
      if (i==0)
      {
          t->port = 0x3F8;    /* COM1 base address */
      }else{
          t->port = 0x2F8;    /* COM2 base address */  
      }
      t->inchars.value = 0;
      //t->outspaces.value = 0;
      //t->outspaces.queue = 0;
      t->inchars.queue = 0;
      
      //t->mutex.value  = 1;
      //t->mutex.queue = 0;
      
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

    int_off();  // CLI; no interrupts

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

    int_on();
    
    enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    /* show greeting message */
    //USE bputc() to PRINT MESSAGE ON THE SERIAL PORT: serial port # ready
  
    while (*q){
      bputc(t->port, *q++);
    }
    
    bputc(t->port, (i+'0'));
    while(*p)
    {
      bputc(t->port, *p++);
    }
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
{ 
  //printf("ignore error\n"); 
  }

int do_modem()
{  
  //printf("don't have a modem\n"); 
}


/* The following show how to enable and disable Tx interrupts */

enable_tx(struct stty *t)
{
  int_off();
  out_byte(t->port + IER, 0x03);
  t->tx_on = 1;
  int_on();
}

disable_tx(struct stty *t)
{ 
  int_off();
  out_byte(t->port + IER, 0x01);
  t->tx_on = 0;
  int_on();
}

/******** echo char to serial port **********/
int secho(struct stty *t, int c)
{
   /* insert c into ebuf[]; turn on tx interrupt */
   t->ebuf[t->ehead] = c;
   t->e_count++;
   t->ehead++;
   t->ehead %= EBUFLEN;
   
   enable_tx(t);
}

int do_rx(struct stty *t)
{ 
  int c;
  c = in_byte(t->port) & 0x7F;  /* read the char from port */
  printf("\nrx interrupt c=%c\n", c);  

  // Put into inBuf
  t->inbuf[t->inhead] = (char)c;
  t->inhead++;
  t->inhead %= INBUFLEN;
  // echo char
  secho(t, c);
  // unblock
  V(&t->inchars);
}      

/*********************************************************************/

int do_tx(struct stty *t)
{
  int c;
  if (t->e_count == 0 && t->outspace.value == 0)// are both empty ==>
  {
    //    turn off tx interrupt; return
    disable_tx(); return;
  }
  
  if (t->e_count != 0) { 
     // out a char from ebuf; return;
    c = t->ebuf[t->etail];
    t->etail++;
    t->etail %= EBUFLEN;
    t->e_count--;
    
    bputc(t->port, (char)c);
    return;
  } 

  if (t->outspace.value != 0){
     // out a char from outbuf; 
     // V any process blocked on outbuf room
    c = t->outbuf[t->outtail];
    t->outtail++;
    t->outtail %= OUTBUFLEN;
    t->outspace.value--;
    
    bputc(t->port, (char)c);
    V(&t->outspace);
    return;
  }
}
     
int sgetc(int port)  // port = 0 or 1 
{ 
    // map port # to stty sturcutre
  struct stty *t = &stty[port];
    // BLOCK if no input char yet
  char c;
    int_off();    
      //get a char c from inbuf[ ]
      c = t->inbuf[t->intail];
      t->intail++;
      t->intail %= INBUFLEN;
    int_on();
    return(c);
}

int sputc(char c, int port)
{
  struct stty *t = &stty[port];
  // map port # to stty pointer
/*    if (t->port != port)
    {
      t = &stty[1];
    }
    */
  // BLOCK if outbuf has no room
    int_off();              
      //enter c into outbuf[ ];
      t->outbuf[t->outhead++] = c;
      t->outhead %= OUTBUFLEN;
      t->outspace.value++;
    int_on();
    //enable tx interrupt if it is off;  
    if(!t->tx_on)
    {
      enable_tx(t);
    }
}

int sgetline(int port, char *line)
{
  struct stty *t = &stty[port];
  
  //char c;
  int i=0;
  
  for(; line[i] != '\0' ; ++i)
  {
    line[i] = sgetc(t);
  }
  
  return i;
}

int sputline(int port, char *line)
{
  struct stty *t = &stty[port];
  int i = 0;
  
  for(; line[i] != '\0' ; ++i)
  {
    sputc(t, line[i]);
  }
}

int ugets(int port, char *y)
{  
  // get a line from serial port and write line to y in U space
  int i, len;
  char line[64];
  printf("UGETS:%d ",port);
  len = sgetline(port, line);
  
  for(i=0; line[i] != '\0'; ++i)
  {
    put_byte(line[i], running->uss, y++);
  } 
  return len;
}

int uputs(int port, char *y)
{
  int i;
  char line[64];
  char c = '0';
  // output the line y in U space to serial port
  printf("Uputs:%d ",port);
  for(i=0; c != '\0' ; ++i)
  {
    c = get_byte(running->uss, y++);
    line[i] = c;
    putc(c);
  }
  
  sputline(port, "outline\n");
  return;
}
