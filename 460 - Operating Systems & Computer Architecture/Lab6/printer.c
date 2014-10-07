#define NPR         1
#define DATA    0x378       // #define PORT 0x3BC  for LPT2
#define STATUS  0x378+1
#define CMD     0x378+2
#define PLEN       64

struct para{
   char pbuf[PLEN];
   int  head, tail;
   SEMAPHORE room;         // any room in pbuf[ ]?
   int isPrinting;         // 1 if printer is printing
   int port;
}printer;                  // printer[NPR] if many printers

int delay(){ }

pr_init()
{  
   //Initialize head, tail, isPrinting to 0;
   //Initialize SEMAPHORE room properly; 
   printf("Printer init()....");
   printer.head = printer.tail = 0;
   printer.isPrinting = 0;
   printer.port = DATA;
   printer.room.value = PLEN;
   printer.room.queue = 0;

   /* initialize printer INIT and SELECT */
   out_byte(DATA+2, 0x08);   /* init */
   out_byte(DATA+2, 0x0C);   /* init, select on */
   
   enable_irq(7);
   printf("Done\n");
}

int phandler()
{
   int status;
   char c;
   printf("printer!");
   status = in_byte(STATUS);    // read status port
   if ( printer.room.value == PLEN)
   {
     // nothing waiting on me. GTFO
     printf("Printer done\n");
     out_byte(CMD, 0x0C);
     printer.tail = 0;
     printer.isPrinting = 0;
     goto out;  // we're done here
   }
   if (status & 0x08){              // check for noERROR only
      // Good to go lets print another item
      c = printer.pbuf[ printer.tail ];
      printer.tail++;
      printer.tail %= PLEN;
      printf("%c tail[%d]\n", c , printer.tail);
      prchar( c );
      //printf("%c\n", c);
      //printer.room.value++;
   }
out:
   out_byte(0x20, 0x20);           // re-enable the 8259
}

/********************** Upper half driver ************************/
int prchar(char c)
{
    //WRITE C CODE for prchar()
    out_byte(DATA, c);
    printer.room.value++;
    //printf("[%c]\n", c);
    out_byte(CMD, 0x1D);     // last bit = 1
	      //  delay(); (optional)
    out_byte(CMD, 0x1C);    // last bit = 0
}

int prline(char *y)
{
  // create and grab all chars from USER space
  char line[128];
  int i=0;
  
  for(i=0;i < 128;++i)
  {
    line[i] = get_byte(running->uss, y++);
    if (line[i] == '\0')
      break;
  }
  printf("prline[%s] len[%d]\n", line, i);
  strcpy(printer.pbuf, line);
  printer.room.value -= i;
  printer.tail = 1;
  prchar(line[0]);
}
