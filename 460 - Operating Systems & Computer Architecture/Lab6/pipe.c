show_pipe(PIPE *p)
{
   int i, j;
   u8 byte;
   printf("------------ PIPE CONTENTS ------------\n");     
   printf("[");
    for(i=0;i<PSIZE;i++){
      if(p->buf[i] == 0){
        putc('.');
      }else{
        putc(p->buf[i]);
      }
    }
   printf("]\n");
   printf("data: %d  ", p->data);
   printf("# writers: %d\n", p->nwriter);
   printf("room: %d  ", p->room);
   printf("# readers: %d\n", p->nreader);
   printf("----------------------------------------\n");
}

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

int pfd()
{
  // print running process' opened file descriptors
  int i, j;
  printf("FD   Ref    Mode   Contents\n");
  printf("--------------------------------\n");
  for ( i =0; i< NFD; ++i)
  {
    if ( running->fd[i]->refCount <= 0)
    {
      continue;
    }
    printf("%d    %d    ", i, running->fd[i]->refCount);
    switch(running->fd[i]->mode){
      case READ_PIPE:
        printf( "READ ");
        break;
      case WRITE_PIPE:
        printf( "WRITE");
        break;
      default: 
        printf( "N/A");
        break;
      }
    printf("  [");
    for(j=0;j<PSIZE;j++){
      if(running->fd[i]->pipe_ptr->buf[j] == 0){
        putc('.');
      }else{
        putc(running->fd[i]->pipe_ptr->buf[j]);
      }
    }
    printf("]\n");
      //printf("  [%s]\n", running->fd[i]->pipe_ptr->buf);
  }
}

PIPE* getPipe()
{
  int i;
  for( i=0;i<NPIPE;++i)
  {
    if ( !pipe[i].busy )
    {
      return &pipe[i];
    }
  }
}

int getOFT(OFT **read, OFT **write)
{
  int i;
  int j=0;
  for ( i =0; i< NOFT;++i)
  {
    if ( oft[i].refCount != 0 )
    {
      continue;
    }
    
    if ( j == 0 )
    {
      printf("READ = oft[%d]: %x\n", i, &oft[i]);
      *read = &oft[i];
      ++j;
      continue;
    }else
    {
      printf("Write = oft[%d]: %x\n", i, &oft[i]);
      *write = &oft[i];
      return 0;
    }
  }
  return -1;
}

int read_pipe(int fd, char *buf, int n)
{
  int 	i, bytes= 0;
  PIPE* pipe;
  u8 	info;
  char  byte;
  printf("fd[%d]  buf[%c]  n[%d]\n", fd, get_byte(running->uss, buf), n );
  
  if(running->fd[fd] ==0){
    printf("fd isn't open\n");
    return -1;
  }

  if(running->fd[fd]->mode != READ_PIPE){
    printf("fd not in read mode\n");
    return -2;
  }
  
  pipe = running->fd[fd]->pipe_ptr;
  //show_pipe(pipe);
  if(pipe->nwriter == 0){
    printf("No Writer\n");
    return -3;
  }

  while ( bytes < n)
  {
    if( pipe->data <= 0)
    {
      kwakeup(&(pipe->room));
      ksleep(&(pipe->data));
    }
    byte = pipe->buf[pipe->tail];
    put_byte(byte, running->uss, buf);
    //printf("Byte[%d]:%c\n", pipe->tail, byte);
    pipe->buf[pipe->tail] = 0;
    pipe->tail++;
    pipe->tail %= PSIZE;
    bytes++;
    pipe->room++;
    pipe->data--;
    buf++;
    kwakeup(&(pipe->room));
  }
  return bytes;
  //tswitch();
  //show_pipe(pipe);
}

int write_pipe(int fd, char *buf, int n)
{
  int 	i, bytes=0;
  PIPE 	*pipe;
  u8 	info;
  
  printf("fd[%d] buf[%c] n[%d]\n", fd, get_byte(running->uss, buf), n);
  
  if(running->fd[fd] ==0){
    printf("fd isn't open\n");
    return -1;
  }

  if(running->fd[fd]->mode != WRITE_PIPE){
    printf("fd not in write mode\n");
    return -2;
  }
  
  pipe = running->fd[fd]->pipe_ptr;
  
  if(pipe->nreader == 0){
    printf("NO READER\n");
    return -3;
  }
  
  //show_pipe(pipe);
  if ( pipe->data == 0)
  {
    pipe->head = 0;
  }
  while( bytes < n )
  {
    if ( pipe->room <= 0 ) 
    {
      kwakeup(&(pipe->data));
      ksleep(&(pipe->room));
    }
    info = get_byte( running->uss, buf);
    //printf("writing %c to %d\n", info , pipe->head);
    pipe->buf[pipe->head] = info;
    pipe->head++;
    buf++;
    pipe->room--;
    pipe->data++;
    bytes++;
    pipe->head %= PSIZE;
    kwakeup(&(pipe->data));
  }
  //show_pipe(pipe);
  tswitch();
  return bytes;
}

int kpipe(int pd[2])
{
  OFT *read, *write;
  PIPE *pipe;
  u16 i = 0, j=0;
  
  pipe = getPipe();
  if( pipe == 0)
  {
    return 0;
  }
  
  if ( getOFT(&read, &write) < 0 )
  {
    printf("No open file descripters\n");
    return -1;
  }
  
  for(i=0;i < NFD;++i)
  {
    if (running->fd[i] == 0)
    {
      printf("Putting [%d] into address %x\n", i, pd);
      put_word(i, running->uss, pd);
      pd++;
      if( j == 0 )
      {
	running->fd[i] = read;
      }else
      {
	running->fd[i] = write;
      }
      ++j;
    }
    if (j == 2)
    {
      break;
    }
  }
  
  read->pipe_ptr = write->pipe_ptr = pipe;
  
  read->mode = READ_PIPE;
  read->refCount = 1;
  printf("READ mode[%d] count[%d]\n", read->mode, read->refCount);
  
  write->mode = WRITE_PIPE;
  write->refCount = 1;
  printf("Write mode[%d] count[%d]\n", write->mode, write->refCount);
  
  for(i=0;i< PSIZE ; ++i)
  {
    pipe->buf[i] = 0;
  }
  
  pipe->head = pipe->tail = pipe->data = 0;
  pipe->nreader = pipe->nwriter = pipe->busy = 1;
  pipe->room = PSIZE;
}

int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  running->fd[fd] = 0;                 // clear fd[fd] entry 

  if (op->mode == READ_PIPE)
  {
    pp = op->pipe_ptr;
    pp->nreader--;                   // dec n reader by 1

    if (--op->refCount == 0){        // last reader
      if (pp->nwriter <= 0){         // no more writers
	    pp->busy = 0;             // free the pipe   
	    return;
      }
    }
    kwakeup(&pp->room); 
    return;
  }
  if( op->mode == WRITE_PIPE)
  {
    pp = op->pipe_ptr;
    pp->nwriter--;                   // dec n writer by 1

    if (--op->refCount == 0){        // last writer
      if (pp->nreader <= 0){         // no more readers
	    pp->busy = 0;             // free the pipe   
	    return;
      }
    }
    kwakeup(&pp->data); 
    return;
  }
}
