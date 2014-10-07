#define LATCH_COUNT	0x00
#define SQUARE_WAVE	0x36

#define TIMER_FREQ	1193182L
#define TIMER_COUNT	((unsigned) (TIMER_FREQ/60))

#define TIMER0		0x40
#define TIMER_MODE	0x43
#define TIMER_IRQ	0

#define INT_CNTL	0x20
#define INT_MASK	0x21

int enable_irq( u16 irq_nr)
{
  //int sr = lock();
  int_off();
  out_byte(INT_MASK, in_byte(INT_MASK) & ~(1 << irq_nr));
  //unlock(sr);
}

int timer_init()
{
  printf("Init Timer\n");
  tick = sec = min = hr = 0;
  out_byte(TIMER_MODE , SQUARE_WAVE);
  out_byte(TIMER0, TIMER_COUNT);
  out_byte(TIMER0, TIMER_COUNT >> 8);
  enable_irq(TIMER_IRQ);
}

int thandler()
{
  int i,r,c;
  PROC* p = sleepList;
  tick++;
  tick %= 60;
  
  if ( tick == 0)
  {
    print_clock();
    
    if(running->inkmode == Kmode)
    {
      running->time--;
      r = row;  c = column;
      row = 23; column = 77;
      printf("%d", running->time);
      row =r; column = c;
      if(running->time == 0)
      {
        //printf("Status[%d]", running->status);
        kbdclear();
        sysKey(0x1C);
        out_byte(0x20, 0x20);
        tswitch();
      }
    }
    
    while(p != 0)
    {
      if( p->time > 0 )
      {
        p->time--;
        if(p->time == 0)
        {
          kwakeup(p->pid);
        }
      }
      p = p->next;
    }
  }
  out_byte(0x20, 0x20);
}

/************Timer Stuff************/
int pause(int t)
{
  //printf("Testing herrro?[%d][%d]\n", running->pid, t);
  if(running->pid <= 1 )
  {
    printf("No naps for you\n");
    return -1;
  }
  if (t <= 1 )
  {
    t = 1;
  }
  running->time = t;
  printf("OK i go sleep now for %d secs\n", t);
  ksleep(running->pid);
}

void printTime(int t)
{
  if(t < 10)
  {
    putc('0');
  }else
  {
    putc( (t/10)+'0' );
  }
  putc( (t%10)+'0');
}

int update_time()
{
  sec++;
  if (sec==60)
  {
    sec = 0;
    min++;
    if(min ==60)
    {
      min =0;
      hr++;
      if(hr == 24)
      {
        hr =0;
      }
    }
  }
}

int print_clock()
{
  int r,c, tmp;
  tmp = color;
  color = DBLACK;
  
  r = row;
  c = column;
  row = 24;
  column = 65;
  printBar();
  running->inkmode > Kmode ? putc('K') : putc('U');
  printf("mode ");
  
  update_time();

  printTime(hr);
  putc(':');
  printTime(min);
  putc(':');
  printTime(sec);
  
  row = r;
  column = c;
  color = tmp;
}
