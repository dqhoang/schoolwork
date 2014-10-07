OFT* falloc()
{
  int i;
  for ( i =0 ; i <NOFT; ++i)
  {
    if( oft[i].refCount == 0)
    {
      return &oft[i];
    }
  }
  return NULL;
}

int fdalloc(OFT *oftp)
{
  oftp->refCount = 0;
  return;
}

int my_mv()
{
  if ( pathname[0] == 0 || parameter[0] ==0)
  { return -1;}
  char *source;
  source = strdup(pathname);
  my_link();
  strcpy( pathname, source);
  my_unlink();
  return;
}

int my_pfd()
{
  /*
  This function displays the currently opened files as follows:
       /a/b/c     1  READ   1234       
  to help the user know what files has been opened.
  */
  int i;
  printf("filename\tfd\tmode\t[dev,ino]\toffset\n");
  printf("--------\t--\t----\t---------\t------\n");
  for ( i =0 ; i < NFD; ++i)
  {
    if( (cur->fd[i]) != NULL)
    {
      printf("%8s\t", cur->fd[i]->name);
      //printf("%8d\t", ( (cur->fd[i])->inodeptr)->ino );
      printf("%2d\t", i);
      //printf("%4d\t", ();
      switch(cur->fd[i]->mode)
      {
	case (0):
	  printf("READ\t");break;
	case 1:
	  printf("WRITE\t");break;
	case 2:
	  printf("RW\t");break;
	case 3:
	  printf("APP\t");break;
      }
      printf("[%3d,%3d]\t", ((cur->fd[i])->inodeptr)->dev, (cur->fd[i])->inodeptr->ino);
      printf("%6d\n", (cur->fd[i])->offset);
    }
  }
}

int truncate(MINODE *mip)
{
  int i;
  unsigned long buff[BLOCK];
  unsigned long block[BLOCK];
/*  1. release mip->INODE's data blocks */
  // DIRECT BLOCKS
  for( i = 0 ; i < 12 && mip->INODE.i_block[i] != 0; ++i )
    dealloc( mip->dev, 0 , mip->INODE.i_block[i] );
  // INDIRECT BLOCKS
  if ( mip->INODE.i_block[i] != 0 )
  {
    get_block(mip->dev, mip->INODE.i_block[i] , buff);
    for( i = 0 ; i < 256 ; ++i )
    {
      dealloc( mip->dev, 0, buff[i] );
    }
  }
  i = 13;
  // DOUBLE INDIRECT
  if ( mip->INODE.i_block[i] != 0 )
  {
    int j=0, c = 0;
    get_block(mip->dev, mip->INODE.i_block[i] , buff);
    while ( j < 256 )
    {
      get_block( mip->dev,block[j], block );
      c = 0;
      while(c < 256)
      {
	if ( buff[c] == 0 ) break;
	dealloc(mip->dev, 0, buff[c] );
	
      }
      if ( buff[c] == 0 ){ break; }
      ++j;
    }
  }
//  2. update INODE's time field
  mip->INODE.i_atime = time(0L);
  mip->INODE.i_mtime = time(0L);
//  3. set INODE's size to 0 and mark Minode[ ] dirty
  mip->INODE.i_size = 0;
  mip->dirty = 1;
//  iput(mip);
}

int my_open()
{
//  1. ask for a pathname and mode to open:
         //You may use mode = 0|1|2|3 for R|W|RW|APPEND
  if ( pathname[0] == 0)
    return -1;
//  2. get pathname's inumber:
  int ino, dev, mode, i;
  MINODE *mip, *pip;
  
  mode = parameter[0] - '0';
  
  if ( mode > 4 || mode < 0 )
    return -1;
  
  if (pathname[0] == '/') 
  { dev = root->dev; }
  else
  { dev = (cur->cwd)->dev; }
  
  ino = getino( pathname );
  //printf("pathname is: %s ino is : %d\n",pathname, ino);
//  3. get its Minode pointer
  mip = iget(dev,ino);
  pip = iget(dev, getino( parent(pathname)) );
//  4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
     //(Optional : do NOT check FILE type so that we can open DIRs for RW)
  
     //Check whether the file is ALREADY opened with INCOMPATIBLE type:
     //      If it's already opened for W, RW, APPEND : reject.
     //      (that is, only multiple R are OK)
  for ( i = 0; i < NOFT ; ++i)
  {
    if ( cur->fd[i] == NULL) 
      continue;
    else if ( cur->fd[i]->inodeptr == mip && ( cur->fd[i]-> mode != 0) )
    {
      return -1;
    }
  }
//  5. allocate an OpenFileTable (OFT) entry and fill in values:
  OFT *oftp;
  oftp = falloc();       // get a FREE OFT
  if ( oftp == NULL )
  {
    printf("OTF is full!\n");
    return;
  }
  oftp->mode = mode;     // open mode 
  oftp->refCount = 1;
  oftp->inodeptr = mip;  // point at the file's minode[]
  //oftp->name = base(pathname);
  strcpy(oftp->name, base(pathname));

//  6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

  switch(mode){
    case 0 : 
      oftp->offset = 0; 
      break;
    case 1 : 
      truncate(mip);        // W : truncate file to 0 size
      oftp->offset = 0;
      break;
    case 2 : 
      oftp->offset = 0;    // RW does NOT truncate file
      break;
    case 3 : 
      oftp->offset =  mip->INODE.i_size;  // APPEND mode
      break;
    default: 
      printf("invalid mode\n");
      return(-1);
  }

//   7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      //Let running->fd[i] point at the OFT entry
  for ( i = 0 ; i < NFD ; ++i)
  {
    if( cur->fd[i] == NULL )
    {
      cur->fd[i] = oftp;
      break;
    }
  }
//   8. update INODE's time field. 
      //for W|RW|APPEND mode : mark Minode[] dirty
  mip->INODE.i_atime = time(0L);
  mip->INODE.i_mtime = time(0L);
  mip->dirty = 1;
//   9. return i as the file descriptor
  return 1;
}

int my_close()
{
  int fd;
  fd = pathname[0] - '0';
  close_file( fd );
  return;
}

int close_file(int fd)
{
//  1. verify fd is within range.
  if ( fd < 0 || fd > NFD )
  {
    return -1;
  }
//  2. verify running->fd[fd] is pointing at a OFT entry
  if ( cur->fd[fd] == NULL )
    return -1;
//  3. The following code segments should be fairly obvious:
  OFT *oftp;
     oftp = cur->fd[fd];
     cur->fd[fd] = 0;
     oftp->refCount--;
     if (oftp->refCount > 0) return 0;
     // last user of this OFT entry ==> dispose of the Minode[]
     MINODE *mip;
     mip = oftp->inodeptr;
     iput(mip);

     //fdalloc(oftp);   (optional, refCount==0 says it's FREE)
     return; 
}

int my_lseek()
{
  int fd;
  long position = (long)atoi(parameter);
  fd = pathname[0] - '0';
  if ( position < 0 ) return -1;
  do_lseek(fd, position);
  return;
}

long do_lseek(int fd, long position)
{
  /*
  From fd, find the OFT entry. 
  change OFT entry's offset to position but make sure NOT to over run
  either end of the file.
  return originalPosition
  */
  long orig;
  OFT *oftp = cur->fd[fd];
  orig = oftp->offset;
  if( position > (oftp->inodeptr)->INODE.i_size)
  {
    return 0;
  }
  oftp->offset = position;
  return orig;
}

int my_read()
{
  /*
  Preparations:
   ask for a fd  and  nbytes to read
   verify that fd is indeed opened for READ or RW
   */
  int nbytes, fd;
  fd = pathname[0] - '0';
  if( fd < 0 || fd > NFD) return -1;
  nbytes = atoi(parameter);
  if ( nbytes < 0 ) return -1;
  char buf[nbytes];
  if ( cur->fd[ fd ]->mode == 0 ||
       cur->fd[ fd ]->mode == 3)
    return(myread(fd, buf, nbytes));
  else 
    return -1;
}

int myread(int fd, char *buf, int nbytes)
{
  char readbuf[BLOCK] ,*cq;
  unsigned long buffer[256];
  
  int size, lbk, start, startByte, blk;
  int fileSize, count, remain;
  OFT *oftp = cur->fd[fd];
  fileSize = (oftp->inodeptr)->INODE.i_size;
  size = fileSize - oftp->offset; //number of bytes remain in file.

  while (nbytes > 0 && size > 0)
  {
     //compute LOGICAL BLOCK lbk and startByte in that block from offset;
       lbk       = oftp->offset / BLOCK;
       startByte = oftp->offset % BLOCK;
     
     // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
     if (lbk < 12){              // direct block
        blk = oftp->inodeptr->INODE.i_block[lbk];
     }
     else if (lbk >= 12 && lbk < 256 + 12) { 
          //  indirect blocks 
        get_block( oftp->inodeptr->dev , oftp->inodeptr->INODE.i_block[12] , buffer);
	blk = buffer[ lbk - 12 ];
     }
     else{ 
       //  double indirect blocks
       int loc = ( lbk - 12 ) / 256 -1 ;
       get_block( oftp->inodeptr->dev , oftp->inodeptr->INODE.i_block[13] , buffer);
       get_block( oftp->inodeptr->dev , buffer[ loc ] , buffer);
       loc = (lbk - 12)%256;
       blk = buffer[ loc ] ;
          
     }
     /* get the data block into readbuf[] */
     get_block(oftp->inodeptr->dev, blk, readbuf);
     /*copy from startByte to buf[], at most remain bytes in this block */
     char *cp = readbuf + startByte;   
     remain = BLOCK - startByte;  // number of bytes remain in readbuf[]

     while (remain > 0){
      *cq++ = *cp++;             // cq points at buf[ ]
      oftp->offset++; 
      count++;                  // count=0 for counting
      size--; nbytes--;  remain--;
      if (nbytes <= 0 || size <= 0) 
	break;
     }
     // if one data block is not enough, loop back to OUTER while for more ...
 }
 printf("myread : read %d char from file %d\n", count, fd);  
 return count;    // count is a actual number of bytes read 
}

int my_write()
{
  int nbytes, fd;
  fd = pathname[0] - '0';
  if( fd < 0 || fd > NFD) return -1;
  nbytes = atoi(parameter);
  if ( nbytes < 0 ) return -1;
  char buf[nbytes];
  if ( cur->fd[ fd ]->mode == 0 ||
       cur->fd[ fd ]->mode == 3)
    return(mywrite(fd, buf, nbytes));
  else 
    return -1;
}

int mywrite( int fd , char *buf, int nbytes)
{/*
  OFT *oftp = cur->fd[fd];
  char readbuf[BLOCK];
  unsigned long buffer[256];
  
  int size, lbk, start, startByte, blk;
  int fileSize;
  
  fileSize = (oftp->inodeptr)->INODE.i_size;
  size = fileSize - oftp->offset; //number of bytes remain in file.
  
  while (nbytes > 0 ){

     compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
          lbk       = oftp->offset / BLOCK_SIZE;
          startByte = oftp->offset % BLOCK_SIZE;

    // I only show how to write DIRECT data blocks, you figure out how to 
    // write indirect and double-indirect blocks.

     if (lbk < 12){                         // direct block
        if (ip->INODE.i_block[lbk] == 0)    // if no data block yet
            mip->INODE.i_block[lbk] = balloc(mip->dev);
        blk = mip->INODE.i_block[lbk];
     }
     else if (lbk >= 12 && lbk < 256 + 12){ 
            // indirect blocks
     }
     else{
            // double indirect blocks //
     }

     // all cases come to here : write to the data block 
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLOCK_SIZE - startByte;  // number of bytes remain in this block

     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (offset > i_size)        // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset>filesize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to while to write more .... until nbytes are written
  }

  mip->dirty = 1;       // mark mip dirty for iput() 
  show ("wrote %d char into file fd=%d\n", nbytes, fd);           
  return nbytes; */
}

int my_cp()
{
  if (pathname[0] == 0 || parameter[0] == 0)
    return -1;
  MINODE *sp, *dp;
  int dev;
  char *source = strdup(pathname);
  char *dest = strdup(parameter);
  char buf[BLOCK];

  
  if( pathname[0] =='/')
    dev = root->dev;
  else
    dev = (cur->cwd)->dev;
  sp = iget( dev, getino(pathname) );
  if( sp == NULL) return -1;
  
  strcpy(pathname, parameter);
  my_creat();
  
  if( dest[0] =='/')
    dev = root->dev;
  else
    dev = (cur->cwd)->dev;
  dp = iget( dev, getino(dest) );
  if ( dp == NULL)
  {
    iput(sp);
    return -1;
  }
  
  // Copy info from source
  dp->INODE.i_mode = sp->INODE.i_mode;
  dp->INODE.i_uid = sp->INODE.i_uid;
  dp->INODE.i_gid = sp->INODE.i_gid;
  dp->INODE.i_size = sp->INODE.i_size;
  dp->INODE.i_blocks = 0; // Will increment as they are allocated
  
  int i;
  // Copy data blocks
  for (i = 0 ; i < 12 && sp->INODE.i_block[i] != 0; ++i)
  {
    dp->INODE.i_block[i] = alloc(dp->dev, 0 );
    if (dp->INODE.i_block[i] < 1) 
    {
      // panic
      iput(sp);
      truncate(dp);
      strcpy( pathname , dest);
      my_unlink;
      printf("Not enough space for copy!\n");
      return;
    }
    bzero(buf, BLOCK);
    get_block( sp->dev, sp->INODE.i_block[i] , buf );
    put_block( dp->dev, dp->INODE.i_block[i] , buf );
    dp->INODE.i_blocks++;
  }
  if ( sp->INODE.i_block[12] != 0 )
  {
    unsigned long sblk[256]; // source
    unsigned long dblk[256]; // destination
    get_block( sp->dev, sp->INODE.i_block[12] , sblk );
    for ( i = 0; i < 256; ++i )
    {
      if ( sblk[i] == 0) { break;}
      dblk[i] = alloc(dp->dev, 0 );
      if (dblk[i] < 1) 
      {
	// panic
	iput(sp);
	truncate(dp);
	strcpy( pathname , dest);
	my_unlink;
	printf("Not enough space for copy!\n");
	return;
      }
      get_block( sp->dev, sblk[i] , buf );
      put_block( dp->dev, dblk[i] , buf );
      dp->INODE.i_blocks++;
    }
    if ( sp->INODE.i_block[13] != 0 )
    { // Triple indirect
    /* NOT DONE
      int j = 0;
      
      get_block( sp->dev, sp->INODE.i_block[13] , sblk );
      for ( j = 0; j< 256 ; ++i)
      {
	get_block( sp->dev, sblk[j] , dblk );
	for ( i = 0; i < 256; ++i )
	{
	  if ( sblk[i] == 0) { break;}
	  dblk[i] = alloc(dp->dev, 0 )
	  if (dblk[i] < 1) 
	  {
	    // panic
	    iput(sp);
	    truncate(dp);
	    strcpy( pathname , dest);
	    my_unlink;
	    printf("Not enough space for copy!\n");
	    return;
	  }
	  get_block( sp->dev, block[i] , buf );
	  put_block( dp->dev, dblk[i] , buf );
	  dp->INODE.i_blocks++;
	}
      } */
    }
  }
  
}