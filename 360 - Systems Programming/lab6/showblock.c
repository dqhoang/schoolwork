int get_block(int dev,int block, char *buf)
{
   lseek(dev,(long)BLOCK*block,0);
   read(dev, buf, BLOCK);
}	

int put_block(int dev, int block, char *buf)
{
   lseek(dev, (long)BLOCK*block,0);
   write(dev, buf, BLOCK);
}

SUPER* superblock( int dev )
{
	SUPER *sp;	// pointer for superblock
	char buf[sizeof(SUPER)]={0}; // buffer to hold raw data read from disk/image	
	// x marks the spot!	
	get_block( dev, SUPERBLOCK , buf);
	//printf("%d %d\n", dev, x);
	// read info in as struct
	sp = (SUPER *)buf;
	// Printing superblock info cause well why the f*ck not
	
	if( view ){
	  printf("=============== SUPER BLOCK ================\n");
	  printf("block_count\t\t %d\n", 	sp->s_blocks_count);
	  printf("inode_count\t\t %d\n", 	sp->s_inodes_count);
	  printf("r block count\t\t %d\n", sp->s_r_blocks_count);
	  printf("#free inodes\t\t %d\n", sp->s_free_inodes_count);
	  printf("#free blocks\t\t %d\n", sp->s_free_blocks_count);
	  printf("first data blk\t\t %d\n", sp->s_first_data_block);
	  printf("magic:\t\t\t %4x\n", 	sp->s_magic);
	  printf("inodes_per_group\t %d\n", sp->s_inodes_per_group);
	  printf("rev_level\t\t %d\n", 	sp->s_rev_level);                 
	  printf("inode size\t\t %d\n",	sp->s_inode_size);
	  /*printf("INODES_PER_BLOCK\t\t %d\n",	INODES_PER_BLOCK);
	  printf("NINODES\t\t %d\n",		NINODES);*/
	  printf("============================================\n");
	}
	
	return sp;
}

GD * group(int dev)
{
	int r;
	char buf[BLOCK]; 
	GD *gb;
	/*
	lseek( dev , BLOCK * GDBLOCK , 0);
	r = read( dev , buf, sizeof(GD) );
	*/
	get_block( dev, GDBLOCK, buf);
	
	gb = (GD *)buf;
	// Print out the group stuff
	if ( view ){
	printf("================== Group ===================\n");
	printf("Bitmap block #\t%d\n",gb->bg_block_bitmap);          // Bmap block number
	printf("Imap block #\t%d\n",gb->bg_inode_bitmap);          // Imap block number
	printf("Inodes Begin #\t%d\n",gb->bg_inode_table);           // Inodes begin block number
	printf("Free inodes #\t%d\n",gb->bg_free_inodes_count);
	printf("Free blocks #\t%d\n",gb->bg_free_blocks_count);     // THESE are OBVIOUS
	printf("Dirs count #\t%d\n",gb->bg_used_dirs_count);
	printf("============================================\n");
	}
	INODEBLOCK = gb->bg_inode_table;
	BBITMAP = gb->bg_block_bitmap;
	IBITMAP = gb->bg_inode_bitmap;
	return gb;
}

char ** strToArray( char * input , int *c)
{
    char *result = NULL;
    char line[256];
    strcpy(line, input);
    if(view)
      printf("Splitting %s!\n", line);
    int i= 0, j=0, argc=0;
    char ** myargv = NULL;
    char delim[] = "/";
    
    // count how many delims we have ( or how many args we have )
    while( line[i] != 0 )
    {
    	if ( line[i] == '/' ) { ++argc; }
    	++i;
    }
    if( line[strlen(line)-1] == '/' )
      argc--;
    if (argc == 0 || line[0] != '/' )
      argc += 1;
      
    if (view) printf("argc is: %d\n", argc);
    myargv = (char **)malloc( argc * sizeof(char *) );
    
    if (argc == 1)
    {	// Only one input
      if ( line[0] == '/') 
	      myargv[0] = &line[1];
      else
	      myargv[0] = line;
      myargv[1] = NULL;
    }else{// Tokenize the string
      result = strtok( line, delim);
      for (i = 0; i < argc; ++i)
      {
	      // allot space for the new string and add it
	      //myargv[i] = (char *)malloc(sizeof(result)+1);
	      //if (view) 
	      myargv[i] = strdup(result);
	      //printf("Result is: %s\n", result);
	      //strcpy(myargv[i] , result);
	      // terminate
	      //strcat(myargv[i], "\0");
	      result = strtok( NULL , delim);
      }
      // make absolute sure the string is terminated
      myargv[i] = NULL;
    }
    *c = argc;
    return myargv;
}

INODE* inode(char* buff)
{
  int c;
  INODE* ip;
  ip = (INODE *)buff + 1 ;
  
  if( view ){
    printf("================== Inode ===================\n");
    printf("Inode mode:\t%x\n",ip->i_mode);			/* File mode */
    printf("Inode UID:\t%d\n", ip->i_uid);				/* Owner Uid */
    printf("Inode Size:\t%d\n", ip->i_size);			/* Size in bytes */
    printf("Inode ID:\t%d\n", ip->i_gid);				/* Group Id */
    printf("Inode Count:\t%d\n", ip->i_links_count);	/* Links count */
    printf("Inode #Blk:\t%d\n",ip->i_blocks);     		/* Blocks count */
    printf("Block 0:\t%d\n",ip->i_block[0]);  			/* Pointers to blocks */
    printf("============================================\n");
  }
  return ip;
}

int search( MINODE* mp, char * name )
{
	int i, c;
	char buff[1024];
	DIR *dp = (DIR *)buff;
	char *cp = buff;            // char pointer pointing at buf[ ]
	
	for ( i = 0 ; i < 12; ++i)
	{
	  if ( mp->INODE.i_block[i] != 0 ) {
	    get_block(mp->dev, mp->INODE.i_block[i] , buff );
	    char n[1024];
	    c = 0; // total counter
	    if(view)
	      printf("Inode\tRec_len\tName_len\tName\n");
	    while(cp < buff + 1024)
	    {
	      strncpy( n , dp->name ,dp->name_len );
	      n[dp->name_len] = '\0';
	      if(view)
		printf("%d\t%d\t%d\t\t%s\n", dp->inode,dp->rec_len, dp->name_len, n);
	      if (!strcmp(name, n ) )
	      {
		if (view)
		  printf("Found %s at %d\n", name , dp->inode);
		return dp->inode;
	      }
	      
	      c += dp->rec_len;
	      if ( c >= 1024 )
		break;
		
	      if(dp->inode == 0){ break;}
	      if(dp->rec_len == 0){break;}
	      cp += dp->rec_len;         // advance cp by rlen in bytes
	      dp = (DIR *)cp;       // pull dp to the next record
	    }
	  }else
	  {
	    if (view )
	      printf("End of readable disks nothing found\n");
	    return -1;
	  }
	}
	return -1;
}

int tst_bit(char *buf, int BIT)
{
  int i, j;
  i = BIT / 8;
  j = BIT % 8;
  return buf[i] & (1 << j);
}

int set_bit(char *buf, int BIT) // set BIT_th bit in char buf[1024] to 1
{
  int i, j;
  i = BIT / 8; 
  j = BIT % 8;
  buf[i] |= (1 << j);
  return 1;
}  
// dev = device
// name = 2d string array of the paths
// ino = ino of the start of the search ( root or cwd )
int getino( char* path)
{
  int i = 0, inum, c = -1, ino, dev;
  
  char buf[BLOCK], **name, current;
  MINODE * mp;
  
  if( ( path[0] == '/' || path[0] == '.' ) && strlen(path) == 1)
    return root->ino;

  if ( path[0] == '/')
  {
    ino = root->ino;
    dev = root->dev;
  }else
  {
    ino = (cur->cwd)->ino;
    dev = (cur->cwd)->dev;
  }
  mp = iget(dev, ino);
  
  name = strToArray(path, &c);
//  printf("name[0]: %s\n", name[0]);
//  current = strdup(name[i]);
  if ( c < 1){ return -1;}
  
  
//  printf("Ino is %d\n", ino);
  i = 0;
  while( name[i] != '\0' ){
    if(view)
      printf("\n------------Searching for name[%d] = %s-----------\n",i, name[i]);
    c = search(mp, name[i] );
    if ( c == 0 )
    {
      printf("Trouble finding %s, Exiting\n", name[i]);
      return(-1);
    }
    iput(mp);
    mp = iget(dev, c);
    if(view)
      printf("\n--------------Found at %d -----------------------\n",c);
    ++i;
  }
  iput(mp);
  return c;
}

int clear_bit(char *buf, int BIT) // clear BIT_th bit in char buf[1024] to 0
{
  int i, j;
  i = BIT / 8; 
  j = BIT % 8;
  buf[i] &= ~(1 << j);
  return 1;
} 
// Update both SUPER and GROUP free inodes
void cFreeInodes(int dev, int inc)
{
  char buf[1024]={0};
  
//Update Super
  get_block( dev, SUPERBLOCK , buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count += inc;
  
  put_block( dev, SUPERBLOCK , buf);
  
// Update GROUP
  get_block( dev, GDBLOCK , buf);
  
  gp = (GD *)buf;
  gp->bg_free_inodes_count += inc;
  put_block( dev, GDBLOCK , buf);
}
// Update both SUPER and GROUP free blocks
void cFreeBlock(int dev, int inc)
{
  char buf[1024]={0};
  
//Update Super
  get_block( dev, SUPERBLOCK , buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count += inc;
  
  put_block( dev, SUPERBLOCK , buf);
  
// Update GROUP
  get_block( dev, GDBLOCK , buf);
  
  gp = (GD *)buf;
  gp->bg_free_blocks_count += inc;
  put_block( dev, GDBLOCK , buf);
}

void dealloc(int dev, int map , unsigned long ino)
{
  int i;  
  char buf[BLOCK];
  if( !map )
  {
    map = BBITMAP;
  }
  else
  {
    map = IBITMAP;
  }
  // get inode bitmap block
  get_block(dev, map, buf);
  clear_bit(buf, ino-1);         // assume you have clr_bit() function 

  // write buf back
  put_block(dev, 4, buf);

  // update free inode count in SUPER and GD
  if ( map == IBITMAP ){
    cFreeInodes(dev,1);         // assume you write this function 
  }else
  {
    cFreeBlock(dev,1);
  }
}

unsigned long alloc(int dev, int map)
{
 int i, blocks;
 char buf[BLOCK];            // BLKSIZE=block size in bytes
 if( !map )
 {
   map = BBITMAP;
   blocks = NBLOCKS;
 }
 else
 {
   map = IBITMAP;
   blocks = NINODES;
 }
 // get inode Bitmap into buf[ ]

 get_block( dev, map , buf );
 for (i=0; i < blocks; i++){  // assume you know ninodes
   if (tst_bit(buf, i)==0){    // assume you have tst_bit() function
     set_bit(buf, i);          // assume you have set_bit() function
     put_block(dev, map, buf);   // write imap block back to disk
     // update free inode count in SUPER and GD on dev
     if(!map)
      cFreeBlock(dev, -1);
     else
      cFreeInodes(dev, -1);       // assume you write this function  
     
     return (i+1);
   }
 }
 return 0;                     // no more FREE inodes
} 

