/**********************************************************************
                         kbd.c file
***********************************************************************/
#define KEYBD            0x60   /* I/O port for keyboard data */
#define PORT_B           0x61   /* port_B of 8255 */
#define KBIT             0x80   /* bit used to ack characters to keyboard */

#define NSCAN             64    /* Number of scan codes */
#define KBLEN             64    // input buffer size

#include "keymap.c"
int sk, ctrl;
extern int color;

typedef struct kbd{           // data struct representing the keyboard
    char buf[KBLEN];          // input buffer
    int  head, tail;          // CIRCULAR buffer
    SEMAPHORE  data;                // number of keys in buf[ ] 
}KBD;

KBD kbd;

int kbd_init()
{
  printf("kbinit()\n");
  //kbd.data = 0;
  
  //SEMAPHORE sem = kbd.data;
  
  kbd.data.value = 0;
  kbd.data.queue = 0;
  
  sk = 0;
  ctrl = 0;
  kbd.head = kbd.tail = 0;
  out_byte(0x20, 0x20);
  printf("kbinit done\n"); 
}

int sysKey( int code )
{
  kbd.buf[kbd.head++] = shift[code];
  kbd.head %= KBLEN;
  kbd.data.value++;
}

int kbdclear()
{
  int i ;
  vidClear();
  
  for( i=0; i < KBLEN ; ++i)
  {
    kbd.buf[i] = 0;
  }
  kbd.head = kbd.tail = kbd.data.value = 0;
}

/************************************************************************
 kbhandler() is the kbd interrupt handler. The kbd generates 2 interrupts
 for each key typed; one when the key is pressed and another one when the
 key is released. Each key generates a scan code. The scan code of key
 release is 0x80 + the scan code of key pressed. When the kbd interrupts,
 the scan code is in the data port (0x60) of the KBD interface. First, read the
 scan code from the data port. Then ack the key input by strobing its PORT_B at
 0x61.
**************************************************************************/

int khandler()
{
  int scode, value, c;
  /* Fetch scan code from the keyboard hardware and acknowledge it. */
  scode = in_byte(KEYBD);        /* get the scan code of the key */
  value = in_byte(PORT_B);       /* strobe the keyboard to ack the char */
  out_byte(PORT_B, value | KBIT);/* first, strobe the bit high */
  out_byte(PORT_B, value);       /* then strobe it low */

  //printf("scode=%x    ", scode);  
  // translate scan code to ASCII; //using shift[ ] table if shifted;
  switch(scode)
  {
    // Shift Key
    case 0x2A:
    case 0x36:
      sk = 1;
      goto out;
    case 0xAA:
    case 0xB6:
      sk = 0; break;
    // Control Key
    case 0x1D:
    case 0xE01D:
      ctrl = 1; goto out;
      
    case 0x9D:
    case 0xE09D:
      ctrl = 0; goto out;
    
    /* Function Buttons*/
    case 0x3B: // f1
      do_ps();
      goto out;
    case 0x3C: 
      break;
    case 0x3D: 
      break;
    case 0x3F: break;
    case 0x40: break;
    case 0x41: break;
    case 0x42: break;
    case 0x43: break;
    case 0x44: break;
    case 0x57: break;
    case 0x58: // F12
      break;
    default:
    if ( sk){
      c= shift[scode];
    }else{
      c = unshift[scode];
    }
  }
  if (scode & 0x80)         // ignore key releases
  {   
    goto out;
  }
  
  /* store the character in buf[ ] for process to get */
  if (kbd.data.value >= KBLEN){   // buf[ ] already FULL
    printf("%c\n", 0x07);
    goto out;          
  }

  kbd.buf[kbd.head] = (char)c;
  kbd.head += 1;
  kbd.head %= KBLEN;

  //kbd.data.value++;           // inc data in buf[] by 1
  //printf("%c: buf[%c] data[%d]\n", (char)c, kbd.buf[kbd.head-1], kbd.data);
  V(&kbd.data);
  //kwakeup(&kbd.data);   // wakeup process in upper half

out:
  out_byte(0x20, 0x20);
}

/********************** upper half routine ***********************/
int getc()
{
  int c;
  int_off();                        // no interrupt 

  while(kbd.data.value <= 0){         
        int_on();                // must unlock beifore going to sleep
        //printf("kbd.data[%d]\n", kbd.data);
        /*
        if ( readyQueue->pid == 0)
        {
          continue;
        }else{
          P(&kbd.data);
        }*/
        //ksleep(&kbd.data);
        int_off();                  // lock again upon woken up
  }

  c = kbd.buf[kbd.tail++] & 0x7F;
  kbd.tail %= KBLEN;
  /*
  if(readyQueue-> pid == 0)
  {
    
  }
  */
  kbd.data.value--;
  //printf("returning %c\n", c);
  int_on();
  return c;
}
/***/
