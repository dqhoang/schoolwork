int copyImage(u16 seg1, u16 seg2, u16 size)
{
  u16 i =0, word=0;
  for(i=0; i < size;++i )
  {
    word = get_word( seg1,i*2);
    put_word( word, seg2, i*2);
  }
}

int fork()
{
  int i,j, pid;  
  u16 segment,info;

  PROC *p = kfork(0);   // kfork() but do NOT load any Umode image for child 
  if (p == 0)           // kfork failed 
    return -1;

  segment = (p->pid+1)*0x1000;
  copyImage(running->uss, segment, 32*1024);
  
  for(j=0;j<10;++j)
  {
    p->kstack[SSIZE - j ] = 0 ;
  }
  p->kstack[SSIZE-1]=(int)goUmode;
  p->inkmode = 1;
  p->ksp = &(p->kstack[SSIZE-9]);
  
  p->uss = segment;
  p->usp = segment - 24;

  reset_segment(p->uss , p->usp);
  
   /**** ADD these : copy file descriptors ***
   for (i=0; i<NFD; i++){
      p->fd[i] = running->fd[i];
      if (p->fd[i] != 0){
          p->fd[i]->refCount++;
          if (p->fd[i]->mode == READ_PIPE)
              p->fd[i]->pipe_ptr->nreader++;
          if (p->fd[i]->mode == WRITE_PIPE)
	      p->fd[i]->pipe_ptr->nwriter++;
      }
   }*/
   return(p->pid);
}

int exec(char *y)
{
  char s[64];
  int i;
  
  u16 segment = (running->pid + 1 ) * 0x1000;
  s[0] = 'b';
  s[1] = 'i';
  s[2] = 'n';
  s[3] = '/';
  for (i=4;i< 64 ;++i)
  {
    s[i] = get_byte(segment, y + (i-4) );
    if ( s[i] == '\0')
      break;
  }
  
  load( s , segment);
  reset_segment(segment);
  
  running->uss = segment;
  running->usp = segment - 24;
  return 1;
}

