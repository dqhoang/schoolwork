// Returns the child name only
char* base(char* path)
{
  if ( path[0] == NULL){ return NULL;}
  char **name;
  int c;
  
  name = strToArray(path, &c);
  if ( c < 1 ) {return NULL;}
  return name[c-1];
}
// Returns the parent path
char* parent(char* path)
{
  if ( path[0] == NULL){ return NULL;}
  char *name, *c;
  
  name = strdup(path);
  c = strrchr(name, '/');
  if ( c == name || c == NULL ) return "/";
  //c = 0;
  name[ c - name] = 0;
  return name;
}
// Decrements mp->refcount, if its 0 then it writes the node back to the disc
void iput( MINODE *mp )
{
  if (view)
  {
    printf("Put Ino:%lu\n", mp->ino);
    pMtbl();
  }
  if (mp == NULL )
    return;
  if ( mp->ino == -1)
    return;
  // decrement ref count
  mp->refCount--;
  if ( mp->refCount > 0 ) return;
  // if rev count is 0 then check if save
  if ( !mp->dirty ) 
  {
    //bzero(mp, sizeof(mp)); 
    clrMP(mp);
    return;
  }
  // if modified is 1 save to disk
  char* buff[BLOCK];
  bzero(buff, BLOCK);
  int blk, disp;
  
  blk = (mp->ino -1 ) / INODES_PER_BLOCK + INODEBLOCK;
  disp = (mp->ino -1 ) % INODES_PER_BLOCK;
  
  lseek( mp->dev , BLOCK * blk+ (128*disp), 0);
  read( mp->dev, buff, BLOCK);
  // structure to structure copy back to disk
  memcpy( buff  , &(mp->INODE), sizeof(INODE));
  
  lseek( mp->dev, BLOCK * blk + (128*disp), 0);
  write( mp->dev, buff, BLOCK);
//  put_block( mp->dev, blk, buff);
  //bzero(mp, sizeof(mp)); 
  clrMP(mp);
}
// Checks mount table for inumber. If it doesn't exist it goes to dev and grabs the inumber
MINODE* iget( int dev, int inumber )
{
  int i;
  // check to see if dev/inumber combo is already loaded
  if( inumber < 2 )  return NULL;
  if (view)
    printf("\nchecking minode table for ino# %d dev:%d\n", inumber, dev);
  
  for( i = 0 ; i < NMINODES ; ++i )
  {
    if( minode[i].dev && view )
      printf("\tMinode[%d]\tdev:%d\tino:%lu\tref:%d\n",i,minode[i].dev,minode[i].ino,minode[i].refCount);
    if ( minode[i].dev == dev && minode[i].ino == inumber && minode[i].refCount > 0 )
    {  
      //printf("Found ino#%d at %d", inumber, i);
      minode[i].refCount++;// increase ref count
      return &minode[i];
    }
  }
  if( view)
    printf("Doesn't exist in ram, searching disk and loading\n");
  // load only if not already in memory
  for ( i = 0 ; i < NMINODES ; ++i)
  {
    if ( minode[i].refCount == 0 )
    {
      MINODE* mp = &minode[i]; 
      int blk, disp;
      char* buff[BLOCK];
      bzero(buff, BLOCK);
      
      mp->dev = dev;
      mp->ino = inumber;
      mp->refCount = 1;
      mp->dirty = 0;
      mp->mounted = 0;
      mp->mountptr = NULL;
      
      blk = ( (inumber-1) / INODES_PER_BLOCK )+ INODEBLOCK;
      disp = (inumber-1) % INODES_PER_BLOCK;
      
      get_block( dev, blk, buff);
      ip = (INODE *) buff + 1;
      ip += (disp -1);
      mp->INODE = *ip;
      if (view){
	printf("ino#%d's block[0] = %d\n", inumber ,mp->INODE.i_block[0]);
	printf("Loaded %d to minode[%d]\n", inumber , i);
      }
      return mp;
    }
  }
  
  printf("No Mo Space in ram\n");
  return NULL;
}
// Funct command. calls pDir on pathname
int my_ls()
{
  MINODE *mp;
  int ino, dev, c, disp=0;
  char **name;
  // set view to 0 (hard to understand with debugs up)
  if (view)
  {
    view = 0;
    disp = 1;
  }
  if ( pathname[0] == "/" ) 
    dev = root->dev;
  else
    dev = (cur->cwd)->dev;

  if ( pathname[0] == 0 )
    ino = (cur->cwd)->ino;
  else
    ino = getino(pathname);
  if ( ino < 2) return -1;
  //printf("ino: %d\n", ino);
  mp = iget(dev, ino );

  pDir( mp );
  iput(mp);
  if (disp)
    view = disp;
  
  return 1;
}
// Util. Called by my_ls Prints info on the directory
int pDir( MINODE *mp )
{
  int i, c, r,j;
  char buff[1024], ch, p[9];//,time[20];
  DIR *dp = (DIR *)buff;
  MINODE *inp;
  char *cp = buff;
  printf("\nPERMISSIONS\tUid\tGid\tIno\tSize\tLinks\tName\t  Modified\n");
  printf(  "-----------\t---\t---\t---\t----\t-----\t----\t  --------\n");
  c = 0; // total counter
  for ( i = 0 ; i < 12; ++i)
  {
    if ( mp->INODE.i_block[i] != 0 ) 
    {
      //printf("block[%d]: %d\n", i , mp->INODE.i_block[i] );
      get_block(mp->dev, mp->INODE.i_block[i] , buff );
      char n[1024];
      cp = buff;
      dp = (DIR*)cp;
      while(cp < buff + 1024)
      {
	if(dp->name == 0){break;}
	// Get the name
	strncpy( n , dp->name ,dp->name_len );
	n[dp->name_len] = '\0';
	// Grab the inode
	inp = iget(mp->dev, dp->inode);
	// Print it out
	do_stat(inp, n);
	// Finished with this one move on
	iput(inp);
	c += dp->rec_len;
	//printf("count is at: %d\n", c);
	if ( c >= mp->INODE.i_size )
	{
	  break;
	}
	if(dp->rec_len == 0){ break;}
	//
	cp += dp->rec_len;         // advance cp by rlen in bytes
	dp = (DIR *)cp;       // pull dp to the next record
      }
    }
  }
  printf("\n");
}
// Funct command. Sets up and calls my_mkdir
int mk_dir()
{  
  int i, c, ino, dev;
  char **paths;
  char *child;
  if( pathname == NULL) return -1;
  if( pathname[0] == '/' && strlen(pathname) == 1) return -1;
  printf("Running mkdir\n");
  MINODE* pip;
  if ( pathname[0] == "/" ) 
    dev = root->dev;
  else
    dev = (cur->cwd)->dev;

  printf("parent into getino %s\n", parent(pathname) );
  ino = getino( parent(pathname) );
  child = base(pathname);
  
  printf("Inumber of parent found: %d\nchild is: %s\n", ino, child);
  if (ino < 2) return -1;
  pip = iget(dev, ino);
/*   Then, verify : 
         parent INODE is a DIR AND
         child des NOT exists in the parent directory;
 */
  if ( pip == NULL )
  {
    printf("We couldn't find the parent?\n");
    return -1;
  }
  else if ( !S_ISDIR(pip->INODE.i_mode) ) 
  {
    iput( pip);
    printf("NOT A DIR!\n");
    return -1;
  }
  else if ( search(pip, child) < -1 )
  {
    iput(pip);
    printf("Child already exists!\n");
    return -1;
  }
  else
  {
    my_mkdir(pip, child);
  }
  iput( pip );
} 
// Makes a directory under PIP named child
int my_mkdir(MINODE* pip, char *child)
{
/*  1. pip points at the parent minode[] of "/a/b", name is a string "c")  */
  int inum, bnum, i;
  MINODE * mp;
  if(view)
    printf("BBITMAP: %d\tIBITMAP %d\n", BBITMAP, IBITMAP);
/*  2. allocate an inode and a disk block for the new directory */
  inum = alloc(pip->dev, 0);
  bnum = alloc(pip->dev, 1);
  if (inum < 1)
    printf("imap alloc failed");
  if (bnum < 1)
    printf("bmap alloc failed");
  if(view)
    printf("Allocated Inode: %d and Bitmap %d for new dir\nBlock:%d\n",inum,bnum, BLOCK);
 
/*  3. load the inode into a minode[] */
  mp = iget(pip->dev, inum);
/* 4. Write contents into mip->INODE */
  mp->INODE.i_mode = 0x41ED;		/* DIR and permissions */
  mp->INODE.i_uid  = cur->uid;	/* Owner Uid */
  mp->INODE.i_gid =  cur->gid;	/* Group Id */
  mp->INODE.i_size = BLOCK;		/* Size in bytes */
  mp->INODE.i_links_count = 2;	/* Links count */
  mp->INODE.i_atime = time(0L);
  mp->INODE.i_ctime = time(0L);
  mp->INODE.i_mtime = time(0L); 
  mp->INODE.i_blocks = 2;     	/* Blocks count in 512-byte blocks */
  
  for( i=0; i < 15 ; ++i)	/* Blocks */
  {  
    mp->INODE.i_block[i] = 0;
  }
  mp->INODE.i_block[0] = bnum; 
  mp->dirty = 1;  /* mark dirty */
  
/* write the new INODE out to disk. */
  if(view)
    printf("Writing new INODE to disk\n");
  if(view)
    printf("INODE uid:%d gid:%d size:%d block[0]:%d\n", mp->INODE.i_uid,mp->INODE.i_gid,mp->INODE.i_size, mp->INODE.i_block[0]);
  iput(mp);  

/*6. Write the . and .. entries into a buf[ ] that is allocated to this directory*/
  char buf[BLOCK];
  bzero( buf, BLOCK);
  DIR * dp;
  dp = (DIR *)buf;
  char *cp=buf;
  
  if(view)
    printf("Creating . and .. for new directory\nInum:%d Bnum:%d\n", inum, bnum);
  dp->inode = inum;		/* Inode number */
  strncpy(dp->name, ".", 1);    /* File name */
  dp->name_len = 1;		/* Name length */
  dp->rec_len = 12;		/* Directory entry length */

  cp += dp->rec_len;            /* advance by rec_len */
  dp = (DIR *)cp;

  dp->inode = pip->ino;      /* Inode number of parent DIR */
  dp->name_len = 2;		/* Name length */
  strncpy(dp->name, "..", 2);   /* File name */
  dp->rec_len = 1012;	/* last DIR entry length to end of block */

  put_block(pip->dev, bnum, buf);
  if (view)
  {
    printf("-----------Verifying directory -------------\n");
    mp = iget(dev, inum);
    pDir( mp );
    iput(mp);
  }
  
/* 7. Finally, enter name into parent's directory */
  int need_length, count=0, nRec, IDEAL_LENGTH;

    // ( ino (8) + name_len + nlen (3) )
  need_length = 4*((8 + strlen(child) + 3)/4);  //a multiple of 4
  
  for ( i=0; i<12; ++i)
  {
    if ( pip->INODE.i_block[i] == 0 )
    {
      pip->INODE.i_block[i] = alloc(pip->dev, 0 );
      bzero( buf, BLOCK);
      dp = (DIR*)buf;
      
      dp->inode = inum;
      dp->rec_len = BLOCK;
      dp->name_len = strlen(child);
      strncpy(dp->name, child, dp->name_len);
      
      put_block(dev, pip->INODE.i_block[i], buf); 
      
      pip->INODE.i_blocks += 2;
      pip->dirty = 1;
      return 1;
    }
    /*   Read parent's data block into buf[]; */
    get_block(pip->dev, pip->INODE.i_block[i], buf);
    cp = buf;
    dp = (DIR*) buf;
    count = 0;
    
    while (cp < buf + BLOCK) 
      {
	IDEAL_LENGTH = 4*( (8 + dp->name_len+3) / 4 );
	count += dp->rec_len;
	if(view)
	{
	  char n[1024];
	  strncpy( n , dp->name ,dp->name_len );
	  n[dp->name_len] = '\0';
	  printf("%d\t%d\t%d\t%s\n", dp->inode,dp->rec_len, dp->name_len,n);
	}
	if (dp->rec_len - IDEAL_LENGTH >= need_length)
	{
	  DIR * prev;
	  nRec = dp->rec_len - IDEAL_LENGTH;
	  prev = dp;
	  prev->rec_len = IDEAL_LENGTH;
	  
	  cp += prev->rec_len;
	  dp = (DIR *)cp;
	  
	  // Set new directory
	  if(view)
	    printf("New name: %s\tInode:%d\tRec_len:%d\n", child, inum,nRec);
	  dp->name_len = strlen(child);
	  dp->inode = inum;
	  dp->rec_len = nRec;
	  strncpy(dp->name, child, dp->name_len);
	  
	  char n[1024];
	  strncpy( n , dp->name ,dp->name_len );
	  n[dp->name_len] = '\0';
	  if(view)
	    printf("Inode:%d\treclen:%d\tnamelen:%d\t%s\n", dp->inode,dp->rec_len, dp->name_len,n);
	  
	  // ok i think we're done so write it back
	  if(view)
	    printf("writing buf back to dev:%d block %d\n", pip->dev, pip->INODE.i_block[i]);
	  put_block(pip->dev, pip->INODE.i_block[i], buf);
	  
	  if(view)
	    printf("Changing Group descriptor\n");
	  get_block(root->dev, GDBLOCK, buf);
	  GD* gd = (GD *)buf;
	  gd->bg_used_dirs_count++;
	  put_block(root->dev , GDBLOCK, buf);
	  if(view)
	    printf("Updating parent\n");
	  
	  // Update parent's info
	  pip->INODE.i_links_count++;
	  pip->INODE.i_atime = time(0L); 
	  pip->dirty = 1;
	  
	  iput(pip);
	  return 1;
	}
	if ( count == 1024 )
	  break;
		
	//if(dp->inode == 0){ break;}
        cp += dp->rec_len;
        dp = (DIR *)cp;
      }
  }
  return -1;
}
// checks to see if a dir is empty
int empty( MINODE* pip )
{
  int nEntry;
  char *cp,buff[1024],  n[256];
  DIR* dp;
  get_block(pip->dev, pip->INODE.i_block[0], buff); 
  // "." 
  cp = buff; 
  dp = (DIR *)cp;
  
  while( cp < buff + BLOCK ){
    bzero(n , 256);
    strncpy(n, dp->name, dp->name_len);
    n[dp->name_len] = 0;
    printf("%s\n", n);
    
    nEntry++;
    if( nEntry > 2)
      return 0;
    
    cp += dp->rec_len;
    dp = (DIR*)cp;
  }
  return 1;
}
// removes an empty directory
int my_rmdir()
{
  int i, ino, dev, c;
  char **name, *child;
  MINODE *pip, *mip = NULL;
  
  // setting pip and mip
  if (pathname[0] == '/') 
  {
    dev = root->dev;
  }
  else
  {
    dev = (cur->cwd)->dev;
  }
  ino = getino(pathname);
  if (ino < 2) return -1;
  mip = iget( dev , ino );
  
  ino = getino( parent(pathname) );
  if (ino < 2) return -1;
  pip = iget( dev, ino );
  
  if ( !( mip->INODE.i_mode & 0040000) == 0040000 ) 
  {
    iput(pip);
    iput(mip);
    printf("NOT A DIR!\n");
    return;
  }

  printf("INO: %d MIP refcount: %d\n",mip->ino, mip->refCount);
  
  if(mip->refCount > 2 || mip->mounted || mip == cur->cwd)
  {
    printf("Sorry that dir is busy\n"); 
    iput(mip);
    iput(pip);
    return; 
    }
  if ( !empty(mip) )
  {
    printf("Dir isn't empty\n");
    iput(pip);
    iput(mip);
    return;
  }

  child = base(pathname);
  printf("child is: %s %d\n", child, pip->ino);
  
  if ( rm_child( pip, child) < 0 )
  {
    printf("rm_child broke\n");
    iput(mip);
    iput(pip);
    return -1;
  }
  
  pip->INODE.i_links_count--; 
  pip->dirty = 1; 
  iput(pip);
  
  for (i=0; i<12; i++)
  {
    if (mip->INODE.i_block[i]==0)
      continue;
    dealloc(mip->dev, 0 ,mip->INODE.i_block[i]);
  }
  dealloc(mip->dev, 1 ,mip->ino);
  iput(mip);
}
// rm_child, removes a directory. 
int rm_child( MINODE *pip, char *c)
{
  char buff[BLOCK], name[256], *ch;
  char *cp, *cpo;
  DIR *dp, *dpo;
  int i, remrec;
  
  
  ch = strdup( base(pathname) );
  printf("rm_child: %s pip:%d\n", ch, pip->ino);
  for ( i = 0 ; i < 12 && pip->INODE.i_block[i] != 0 ; ++i)
  {
    get_block( pip->dev, pip->INODE.i_block[i], buff);
    cp = buff;
    dp = (DIR*)cp;
    
    while( cp < buff + BLOCK)
    {
      bzero( name , 256);
      strncpy(name, dp->name, dp->name_len); 
      name[dp->name_len] = NULL; 
      if( view)
      {
	printf("checking %s against %s rec:%d\n", ch, name, dp->rec_len); 
      }
      if ( !strcmp( ch, name) )
      {
	      if( dp->rec_len == BLOCK ) // new block
	      {
	        dealloc( pip->dev, 0 , pip->INODE.i_block[i] );
	        pip->INODE.i_blocks -= 2;
	        pip->dirty =1;
	      }else
	      {
		//int prevRec = dp->rec_len;
	        //dpo = (DIR*)dp; // DPO is now previous dir
	        remrec = dp->rec_len;
	        cpo = cp; 
	        
	        cp += dp->rec_len;
	        dp = (DIR *)cp;
	        
		if ( cp >= buff + BLOCK)
		{
		  dpo->rec_len += remrec;
		  put_block(pip->dev, pip->INODE.i_block[i], buff);
		  pip->dirty = 1;
		  return 1;
		}else
		{
		  memcpy( cpo, cp, buff+ BLOCK -cp );
		}
	        dpo = (DIR*)cpo;
	        while( cpo + dpo->rec_len + remrec < buff + BLOCK )
	        {
	          cpo += dpo->rec_len;
	          dpo = (DIR *)cpo;
	        }
	        
	        dpo->rec_len += remrec;
	        put_block(pip->dev, pip->INODE.i_block[i], buff);
		pip->dirty = 1;
	        return 1;
	      }
	
      }
      cp += dp->rec_len; 
      dpo = dp;
      dp = (DIR *)cp; 
    }
  }
  return -1;
}
// creats a link to old file with new name
int my_link()
{
  char *source, *dest, buf[BLOCK], *cp, n[256], **name;
  DIR* dp;
  int dev = (cur->cwd)->dev, i, c, sino, dino;
  MINODE *sip, *dip;
  
  if ( pathname == NULL || parameter == NULL )
  {
	  printf("Invalid input,format is: link source destination\n");
	  return;
  }
  
  source = strdup(pathname);
  dest = strdup(parameter);
  printf("Linking %s to %s\n", source, dest);

  sino = getino(source);
  printf("sino: %d\n", sino);
  if (sino < 2) return -1;
  sip = iget( dev, sino);
  
  if ( sip == NULL){printf("couldn't find source!ino :%d\n" , sino);return;}
  if ( S_ISDIR(sip->INODE.i_mode)){printf("Cant link dirs\n"); iput(sip);return;}

  dest = base(parameter);
  dino = getino(parent(parameter) );
  printf("Dino: %d dest is %s\n", dino, dest);
  if (dino < 2) return -1;
  dip = iget(dev, dino );
  if ( dip == NULL){
    printf("couldn't find Destination parent!\n");
    iput(sip);
    return;
  }
  if ( !S_ISREG(sip->INODE.i_mode)){
    printf("Source must be a reg\n"); 
    iput(sip); 
    iput(dip);
    return;
  }
  if ( search(dip, dest) < -1 ){
    iput(dip);
    iput(sip);
    printf("Destination %s already exists!\n", dest);
    return;
  }
  // Ok all setup lets create this file
  printf("Creating %s in ino: %d\n", dest, dip->ino);
  mk_creat(dip,dest);
  
  for(i=0; i<12 ; ++i)
  {
    if ( dip->INODE.i_block[i] == 0)
    {
      dip->INODE.i_block[i] = alloc( dip->dev, 0);
      bzero( buf, BLOCK);
      
      cp = buf;
      dp = (DIR*)cp;
      
      dp->rec_len = BLOCK;
      dp->inode = sino;
      dp->name_len = strlen(dest);
      strncpy(dp->name, dest, dp->name_len);
      put_block(dip->dev, dip->INODE.i_block[i], buf);
      
      dip->INODE.i_size += BLOCK;
      sip->INODE.i_links_count++;
      dip->INODE.i_atime = time(0L);
      dip->INODE.i_mtime = time(0L);
      dip->INODE.i_blocks += 2;
      sip->dirty = 1;
      dip->dirty = 1;
      iput(dip);
      iput(sip);
      return;
    }
    get_block(dev, dip->INODE.i_block[i], buf);
    cp = buf;
    dp = (DIR *)cp;
    while( cp < buf + BLOCK)
    {
      strncpy(n, dp->name , dp->name_len);
      n[dp->name_len] = NULL;
      if( !strcmp(n, dest))
      {
	// deallocate the new inode
	dealloc( dip->dev, 1, dp->inode);
        dp->inode = sino;
        put_block(dev, dip->INODE.i_block[i], buf);
	iput(dip);
  
	sip->INODE.i_links_count++;
	sip->dirty = 1;
	iput(sip);
	return;
      }
      cp += dp->rec_len;
      dp = (DIR*)cp;
    }
  }
  iput(dip);
  
  sip->INODE.i_links_count++;
  sip->dirty = 1;
  iput(sip);
}
// removes a file and decrements inode link count
int my_unlink()
{
  int i , c , sino;
  MINODE *mp;
  char *child;
  if (pathname == NULL )
  {
    printf("Needs path\n");
  }
  
  if (pathname[0] == '/') 
  {
    dev = root->dev;
  }
  else
  {
    dev = (cur->cwd)->dev;
  }
  sino = getino(pathname);
  if (sino < 2) return -1;
  mp = iget(dev, sino );
  if (mp == NULL){ printf("Couldn't find file\n"); return;}
  if ( S_ISDIR(mp->INODE.i_mode) ){ printf("Can't be a file\n"); iput(mp); return;}
  
  // ok lets get into the meat of it
  mp->INODE.i_links_count--;
  
  if ( mp->INODE.i_links_count < 1)
  { // No more links, lets get rid of it
    for ( i = 0; i < 12; ++i)
    {
      dealloc(mp->dev, 0 , mp->INODE.i_block[i]); // Bitmap dealloc
    }
    dealloc(mp->dev, 1 , mp->ino); // Inode dealloc
  }
  // ok done with child, dirty and put it back
  mp->dirty =1 ;
  iput(mp);
  
  // Remove it from parent
  mp = iget( (cur->cwd)->dev , getino( parent(pathname) ) );

  child = base(pathname);
  printf("Child before rmchild %s\n", child);
  // removing from parent
  rm_child( mp, base(pathname) );
  iput(mp);
}
// creates a file that links to source file
int my_symlink()
{
  int sino, dev, dino, i,c;
  MINODE *mp;
  char buf[BLOCK];
  char *source, *dest, **name;
  
  source = strdup(pathname);
  dest = strdup(parameter);
  
  if (pathname[0] == '/') 
  {
    dev = root->dev;
  }
  else
  {
    dev = (cur->cwd)->dev;
  }
  
  //verify source exists
  sino = getino(source);
  if(sino < 2 ) {
    printf("Source does not exist.\n");
    return; 
  }

  strcpy( pathname, dest);
//creat a FILE for dest
  my_creat();
  
//  name = strToArray(dest,&c);
  dino = getino(dest);
  mp = iget(dev, dino);
  if(dino == 0) {
    printf("New linked filed not created\n");
    return;
  }
  
  mp->INODE.i_mode = LINK_MODE;
  mp->dirty = 1;
  
  get_block(dev, mp->INODE.i_block[0], buf);
  
  write(buf, source, strlen(source));
  
  put_block(dev, mp->INODE.i_block[0], buf);

  iput(mp);
}
// Sets up do_stat using pathname as input on single file
int my_stat()
{
  printf("************* Stat *************\n");
  MINODE *mp;
  int ino, dev, c, disp=0;
  char *child, buf[BLOCK];
  
  if( pathname == NULL || pathname[0] == 0)
  { // nothing passed in
    return;
  }
  else { 
      ino = getino(pathname);
    }
  if ( ino < 2 ) return -1;
  
  if (pathname[0] == '/') 
  {
    dev = root->dev;
  }
  else
  {
    dev = (cur->cwd)->dev;
  }
  if (ino < 2) return -1;
  mp = iget(dev, ino);
  child = base(pathname);
  printf("ino %d child %s block0: %d\n",mp->ino, child, mp->INODE.i_block[0]);
  if( mp == NULL ) return -1;
  if(view)
  {
    printf("************ Sending to DS **************\n");
    printf("Name: %s\t ino:%d\n", child, mp->ino);
    }
  do_stat(mp, child);
  iput(mp);  
}
// inp is the minode of the file/dir that we want to print out, name is the name of the item
int do_stat(MINODE *inp, char* name)
{
	if(view)
	{
	  printf("************ do stat **************\n");
	  printf("Name: %s\t ino:%d\n", name, inp->ino);
	}
	char p[9], c, time[26];
	int i,j;
	bzero(time,26);
	bzero(p,9);
	i = 0;
	j = 8;
	while(i <= 8){
	  if(i == 8 || i == 5 || i == 2){ c = 'r';}
	  if(i == 7 || i == 4 || i == 1){ c = 'w';}
	  if(i == 6 || i == 3 || i == 0){ c = 'x';}

	if ( (inp->INODE.i_mode & (1 << i) ) )
	{
	    p[j] = c;
	}else{
	    p[j] = '-';
	}
	--j;
	++i;
	}
	p[9] = 0;
	if( ( inp->INODE.i_mode & 0120000) == 0120000 )
	{  printf("L ");}
	else if( ( inp->INODE.i_mode & 0100000) == 0100000 )
	{  printf("R ");}
	
	 if( ( inp->INODE.i_mode & 0040000) == 0040000 )
	{  printf("D ");}
	
	
	printf("%s\t", p);   
	printf("%3d\t", inp->INODE.i_uid);
	printf("%3d\t", inp->INODE.i_gid);
	printf("%3d\t", inp->ino);
	printf("%d\t", inp->INODE.i_size);
	printf("%d\t", inp->INODE.i_links_count);
	printf("%s\t", name);
	ctime_r( &(inp->INODE.i_mtime), time);
	time[strlen(time)-1] = 0;
	printf("%26s\n", time );
}
// Changes permissions to given
int my_chmod()
{
  /*
    Permissions:
    ME, Group , Everyone
    7 - rwx - 111
    6 - rw- - 110
    5 - r-x - 101
    4 - r-- - 100
    0 - --- - 000
    */
  if ( parameter[0] == 0 || strlen(pathname) != 4 )
  {
    printf("Invalid input");
    return;
  }
  MINODE *mp;
  int perm = 0;
  int i,ino, dev, c, exp;
  char **name, *cp;
  
  if ( parameter[0] == '/' )
  { 
    dev = root->dev; 
  }
  else { 
    dev = (cur->cwd)->dev; 
  }
  
  ino = getino( parameter );
  
  if (ino < 2) return -1;
  
  mp = iget( dev, ino );
  cp = pathname;
  for ( i = 0 ; i < 4; ++i)
  {
    exp = 1;
    for ( c = 0; c < ( 3-i ) ;++c)
    {
       exp *= 8;
    }
    perm += ( pathname[i] - '0' ) * exp ;
  }
  if ( S_ISDIR(mp->INODE.i_mode) )
  { 
    perm += 16384;
  }
  else /*( S_ISREG(mp->INODE.i_mode)) */// rest will be reg for now
  { perm += 32768;}
  mp->INODE.i_mode = perm;
  mp->dirty = 1;
  iput(mp);
}
// Changes the owner id
int my_chown()
{
  if ( parameter[0] == 0 || pathname[0] == 0 )
  {
    printf("Invalid input");
    return;
  }
  
  int new = 0;
  MINODE *mp;
  int ino;
  
  ino = getino( parameter);
  mp = iget( (cur->cwd)->dev , ino);
  
  sscanf(pathname, "%d", &new );
  printf("New user %d\n", new );
  
  mp->INODE.i_uid = new;
  mp->dirty = 1;
  iput(mp);
}
// Changes the Group id
int my_chgrp()
{
  if ( parameter[0] == 0 || pathname[0] == 0 )
  {
    printf("Invalid input");
    return;
  }
  
  int new = 0;
  MINODE *mp;
  int ino;
  
  ino = getino( parameter);
  mp = iget( (cur->cwd)->dev , ino);
  
  sscanf(pathname, "%d", &new );
  printf("New group %d\n", new );
  
  mp->INODE.i_gid = new;
  mp->dirty = 1;
  iput(mp);
}
// Changes the working directory
int my_cd()
{
  int c=0, ino, i, dev;
  char **name;
  MINODE *mp;
  // no path going home
  if ( pathname[0] == 0 )
  {
    iput(cur->cwd);
    cur->cwd = iget(root->dev, root->ino);
    return;
  }
  
//  name = strToArray(pathname , &c);
  // only / was given, going home
  if ( strlen(pathname) == 1 && pathname[0] == '/')
  {
    iput(cur->cwd);
    cur->cwd = root;
    return;
  }
  
  if (pathname[0] == '/') 
  {
    dev = root->dev;
  }
  else
  {
    dev = (cur->cwd)->dev;
  }

  ino = getino(pathname);
  //printf("%d\n", ino);
  if ( ino < 2) return -1;
	
  mp = iget(dev, ino);
  
  if (mp == NULL) return -1;
  if ( !S_ISDIR(mp->INODE.i_mode) ) 
  {
    printf("NOT A DIR!\n");
    return -1;
  }
  iput(cur->cwd);
  cur->cwd = mp;
  return;
}

// Util function, returns the pwd string
char* do_pwd()
{
  char name[128], tmp[128], buf[BLOCK], *cp;
  int ino, dev;
  unsigned long myino;
  MINODE *mp;
  DIR *dp;
  if ( cur->cwd == root)
    return "/";
  
  mp = iget( (cur->cwd)->dev, (cur->cwd)->ino);
  if ( mp == NULL) return -1;
  if (view)
    printf("*************PWD!!***************\nIblock[0]:%d \n",mp->INODE.i_block[0] );
  dev= mp->dev;
  bzero(name, 128);
  bzero(tmp, 128);
  while(mp->ino != 2){
    // Get the 0 block to get the ..
    get_block( mp->dev, mp->INODE.i_block[0] , buf);
    ino = mp->ino;
    cp = buf;
    dp = (DIR *)cp;
    if(view)
      printf("This: Inode:%d Rec:%d Name:%d\n", dp->inode,dp->rec_len, dp->name_len);
    cp += dp->rec_len;
    dp = (DIR *)cp; // move to ..
    if(view)
      printf("Parent: Inode:%d Rec:%d Name:%d\n", dp->inode,dp->rec_len, dp->name_len);
    // get the parent's mp
    iput(mp);
    mp = iget( dev, dp->inode);
    
    bzero(tmp, 128);
    strcpy(tmp, "/");
    strcat(tmp,findmyname(mp, ino));
    strcat(tmp, name); // put tmp back on the name
    strcpy(name, tmp);
    if (view)
      printf("PWD NAME:%s\n",name);
  }
  return name;
}
// Prints the absolute path of the directory
int my_pwd()
{
  printf("%s\n", do_pwd() );
}
/*
 * Given the parent DIR (MINODE pointer) and my inumber, this function finds 
 * the name string of myino in the parent's data block.
*/
char *findmyname(MINODE *mp, unsigned long myino) 
{
  int i, count;
  char buff[BLOCK], n[128];
  DIR *dp = (DIR *)buff;
  char *cp = buff;
  if (view)
    printf("************FMN!**************\n");
  for ( i =0 ; i < 12; ++i)
  {
    if ( mp->INODE.i_block[i] != 0 )
    {
      get_block(mp->dev, mp->INODE.i_block[i] , buff);
      count = 0;
      cp = buff;
      dp = (DIR *)cp;
      
      while ( cp < buff + BLOCK)
      {
	if(view)
	  printf("dp->ino:%d and myino: %d\n", dp->inode , myino);
	if ( dp->inode == myino )
	{
	  if(view)
	    printf("FOUND IT\n");
	  bzero(n, 128);
	  strncpy( n , dp->name ,dp->name_len );
	  n[dp->name_len] = 0;
	  if(view)
	    printf("%s\n", n);
	  return n;
	}
	count += dp->rec_len;
	if ( count >= 1024 )
	  break;
	if(dp->inode == 0){ break;}
	if(dp->rec_len == 0) {break;}
	cp += dp->rec_len;         // advance cp by rlen in bytes
	dp = (DIR *)cp;       // pull dp to the next record
      }
    }
  }
  return NULL;
}
// sets up my_creat
int my_creat()
{
  printf("Creat\n");
  
  int i, c, ino, dev;
  char *child;
  if( pathname == NULL) return -1;
  if( pathname[0] == '/' && strlen(pathname) == 1) 		return -1;
  MINODE* pip;
  
  if (pathname[0] == '/') 
       dev = root->dev;
  else
       dev = (cur->cwd)->dev;
  
  printf("to getino %s\n", parent(pathname) );
  ino = getino( parent(pathname) );
  child = base(pathname);
  if (ino < 0) return -1;
  printf("Inumber of parent found: %d\nchild is: %s\n", ino, child);
  pip = iget(dev, ino);
  
  if ( pip == NULL )
  {
    printf("We couldn't find the parent?\n");
    return -1;
  }
  else if( search(pip, child) > -1 )
  {
    printf("Child already exists!\n");
    return -1;
  }
  else
  {
    mk_creat(pip, child);
  }
  iput(pip);
} 
// Creates a file
int mk_creat( MINODE *pip, char *child)
{
  if(view)
    printf("******************* MK CREAT*********************\n");
    int inum,i;
    MINODE *mp;
    
    inum = alloc( pip->dev, 1);
    if(inum<1)
    {
      printf("allocation failed\n");
      return -1;
    }
    
    if (view)
      printf("allocated Inum:%d for %s\n", inum, child);
    mp = iget(pip->dev, inum);
    // Messing with inode
    mp->INODE.i_mode = FILE_MODE;		/* DIR and permissions */
    mp->INODE.i_uid  = cur->uid;	/* Owner Uid */
    mp->INODE.i_gid =  cur->gid;	/* Group Id */
    mp->INODE.i_size = 0;		/* Size in bytes */
    mp->INODE.i_links_count = 1;	/* Links count */
    mp->INODE.i_atime = time(0L);
    mp->INODE.i_ctime = time(0L);
    mp->INODE.i_mtime = time(0L); 
    mp->INODE.i_blocks = 0;     	/* Blocks count in 512-byte blocks */
    
    for( i=0; i < 15 ; ++i)
      mp->INODE.i_block[i] = 0;
    mp->dirty = 1;
    // Ok done
    iput(mp);
    
    // Adding entry into parent's block
    char buf[BLOCK], *cp;
    DIR *dp;
    int need_length, count=0, nRec, IDEAL_LENGTH;
    need_length = 4*((8 + strlen(child) + 3)/4);
    
    for ( i = 0 ; i < 12 ; ++i)
    {
      if ( pip->INODE.i_block[i] == 0 )
      {
	pip->INODE.i_block[i] = alloc(pip->dev, 0 );
	bzero( buf, BLOCK);
	dp = (DIR*)buf;
	
	dp->inode = inum;
	dp->rec_len = BLOCK;
	dp->name_len = strlen(child);
	strncpy(dp->name, child, dp->name_len);
	
	put_block(dev, pip->INODE.i_block[i], buf); 
	
	pip->INODE.i_blocks += 2;
	pip->dirty = 1;
	return 1;
      }
      get_block(pip->dev, pip->INODE.i_block[i], buf);
      cp = buf;
      dp = (DIR*)buf;
      while( cp < buf + BLOCK)
      {
	IDEAL_LENGTH = 4*((8 + dp->name_len+ 3)/4);
	count += dp->rec_len;
	if(view)
	{
	  char n[1024];
	  strncpy( n , dp->name ,dp->name_len );
	  n[dp->name_len] = '\0';
	  printf("%d\t%d\t%d\t%s\n", dp->inode,dp->rec_len, dp->name_len,n);
	}
	if( dp->rec_len - IDEAL_LENGTH >= need_length)
	{
	  nRec = dp->rec_len - IDEAL_LENGTH;
	  dp->rec_len = IDEAL_LENGTH;
	  
	  cp += dp->rec_len;
	  dp = (DIR *)cp;
	  // Set new file
	  if(view)
	    printf("New name: %s\tInode:%d\tRec_len:%d\n", child, inum,nRec);
	  dp->name_len = strlen(child);
	  dp->inode = inum;
	  dp->rec_len = nRec;
	  strncpy(dp->name, child, dp->name_len);
	  
	  if(view)
	    printf("writing buf back to dev:%d block %d\n", pip->dev, pip->INODE.i_block[i]);
	  put_block(pip->dev, pip->INODE.i_block[i], buf);
	  
	  return;
	}
	count+=dp->rec_len; 
	if(dp->inode == 0){ break;}
	if(dp->rec_len == 0){break;}
        cp += dp->rec_len; 
        dp = (DIR *)cp; 
      }
    }
}  

// Changes the modified time of the file, makes it if it doesn't exist?
int my_touch()
{
  if ( pathname == NULL || pathname[0] == 0)
  {
	  printf("Need input\n");
	  return;
  }
  char **name;
  int i, ino, dev, c;
  MINODE *mp;
//  dev = (cur->cwd)->dev;
  if (pathname[0] == '/' )
  {
	  dev = root->dev;
  }else{ dev = (cur->cwd)->dev;}
//  name = strToArray(pathname, &c);
  ino = getino(pathname);
  if ( ino < 0 ) // create file
  {
    printf("Doesn't exist making file\n");
	  my_creat();
	  
	  mp = iget( dev, getino(pathname) );
	  if ( mp == NULL )
	  {
		  printf("Couldn't make the file\n");
		  return;
	  }
  }else
  {
    mp = iget( dev, ino);
  }
  
  mp->INODE.i_atime = time(0L);
  mp->INODE.i_mtime = time(0L);
  mp->dirty =1;
  iput(mp);
  }
// Cleans up minode table and clean exits
int quit()
{
	int i;
      //iput all DIRTY minodes before shutdown
      //iput(root);
	pMtbl();
      for ( i =0; i < NMINODES ; ++i)
      {
      	while( minode[i].refCount > 0)
      		iput(&minode[i]);
      } 
      exit(100);
}
