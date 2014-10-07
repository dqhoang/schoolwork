/************* VIDEO DRIVER vid.c file of MTX kernel *********************/
// vid_init() initializes the display org=0 (row,col)=(0,0)
int vid_init()
{ 
  int i, w;
  org = row = column = 0;
  color = HYELLOW;

  set_VDC(CUR_SIZE, CURSOR_SHAPE);    // set cursor size        
  set_VDC(VID_ORG, 0);                // display origin to 0
  set_VDC(CURSOR, 0);                 // set cursor position to 0

  // clear screen
  w = 0x0700;    // White, blank char // attribute byte=0000 0111=0000 0RGB
  for (i=0; i<25*80; i++)
      put_word(w, base, 0+2*i);       // write 24*80 blanks to vRAM
}

int vidClear()
{
  int i , co = color;
  color = DBLACK;
  for(i = 0; i< SCR_LINES ; ++i)
  {
    scroll();
  }
  color = co;
  printBar();
  row =0; column =0;
}

/*************************************************************************
 scroll(): scroll UP one line
**************************************************************************/
int scroll()
{
  int i;
  u16 w, bytes;  

  // test offset = org + ONE screen + ONE more line
  offset = org + SCR_BYTES + 2*LINE_WIDTH;
  if (offset <= vid_mask){   // offset still within vRAM area
    org += 2*LINE_WIDTH;     // just advance org by ONE line
  }
  else{  // offset exceeds vRAM area ==> reset to vRAM beginning by    
         // copy current rows 1-24 to BASE, then reset org to 0
    for (i=0; i<24*80; i++){
      w = get_word(base, org+160+2*i);
      put_word(w, base, 0+2*i);
    }  
    org = 0;
  }
  // org has been set up correctly
  offset = org + 2*(SCR_LINES)*LINE_WIDTH;   // offset = beginning of row 24

  // copy a line of BLANKs to row 24
  w = 0x0C00;  // HRGB=1100 ==> HighLight RED, Null char
  for (i=0; i<LINE_WIDTH; i++)                  
    put_word(w, base, offset + 2*i);
  set_VDC(VID_ORG, org >> 1);   // set VID_ORG to org     
}

int scrollUp()
{
  org -= 2*LINE_WIDTH;
  set_VDC(VID_ORG, org >> 1 );
}

/***************************************************************
    With the video driver, this is the only putc() in MTX
***************************************************************/
// display a char, handle special chars '\n','\r','\b'
int putc(char c)  
{
  int pos, w, offset;

  if (c=='\n'){
    row++;
    if (row>=24){
      row = 23;
      scroll();
    }
    pos = 2*(row*LINE_WIDTH + column);
    offset = (org + pos) & vid_mask;
    set_VDC(CURSOR, offset >> 1);
    return; 
  }
  if (c=='\r'){
     column=0;
     pos = 2*(row*LINE_WIDTH + column);
     offset = (org + pos) & vid_mask;
     set_VDC(CURSOR, offset >> 1);
     return;
  }
  if (c=='\b'){
    if (column > 0){
      column--;
      pos = 2*(row*LINE_WIDTH + column);
      offset = (org + pos) & vid_mask;
      put_word(0x0700, base, offset);
      set_VDC(CURSOR, offset >> 1); 
    }
    return;
  }
  // c is an ordinary char
  pos = 2*(row*LINE_WIDTH + column);  
  offset = (org + pos) & vid_mask;
  w = color;
  w = (w << 8) + c;
  put_word(w, base, offset);
  column++;
  if (column >= LINE_WIDTH){
    column = 0;
    row++;
    if (row>=24){
      row = 23;
      scroll();
    }
  }
  // calculate new offset
  pos = 2*(row*LINE_WIDTH + column);
  offset = (org + pos) & vid_mask;
  set_VDC(CURSOR, offset >> 1);
}     

/*===========================================================================*
 *                              set_VDC                                      *
 *===========================================================================*/
int set_VDC(u16 reg, u16 val) // set register reg to val
{
  int SR;
  SR = lock();                  /* try to stop h/w loading in-between value */
  out_byte(VDC_INDEX, reg);     /* set the index register */
  out_byte(VDC_DATA,  (val>>8) & 0xFF); /* output high byte */
  out_byte(VDC_INDEX, reg + 1); /* again */
  out_byte(VDC_DATA,  val&0xFF);        /* output low byte */
  unlock(SR);
}
/********************* end if vid.c file ***********************************/
