#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/stat.h>

int fd=0, InodesBeginBlock=0, blk=0, disp=1, ipb=0, size=0;

struct ext2_inode *ip;

int superblock()
{
	struct ext2_super_block *sp;	// pointer for superblock
	char buf[sizeof(struct ext2_super_block)]={0}; // buffer to hold raw data read from disk/image	
	// x marks the spot!	
	lseek( fd , 1024 ,0 );
	// Grab the data
	read( fd, buf, sizeof(struct ext2_super_block) );
	//printf("%d %d\n", fd, x);
	// read info in as struct
	sp = (struct ext2_super_block *)buf;
	// Printing superblock info cause well why the f*ck not
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
	printf("============================================\n");
	
	ipb = ( 1024*(1 << sp->s_log_block_size) ) / sp->s_inode_size ; // blksize / inode size
	//Ensure that disk is ext2 here ... somehow	
	return ( sp->s_magic == 0xEF53 ) ;
}

int group()
{
	int r;
	char buf[sizeof(struct ext2_group_desc)]={0}; 
	struct ext2_group_desc *gb;
	
	// 1024 * 2 because its the second block and i don't math 
	lseek( fd , 1024*2 , 0);
	r = read( fd , buf, sizeof(struct ext2_group_desc) );
	
	gb = (struct ext2_group_desc *)buf;
	// Print out the group stuff
	printf("================= Group %d ==================\n", (r/32)-1 );
	printf("Bitmap block #\t%d\n",gb->bg_block_bitmap);          // Bmap block number
	printf("Imap block #\t%d\n",gb->bg_inode_bitmap);          // Imap block number
	printf("Inodes Begin #\t%d\n",gb->bg_inode_table);           // Inodes begin block number
	InodesBeginBlock = gb->bg_inode_table;
	printf("Free blocks #\t%d\n",gb->bg_free_blocks_count);     // THESE are OBVIOUS
	printf("Free inodes #\t%d\n",gb->bg_free_inodes_count);
	printf("Dirs count #\t%d\n",gb->bg_used_dirs_count);
	printf("============================================\n");

	return 1;
}

char ** strToArray( char * input , int *c)
{
	char *result = NULL;
	char line[256];
	strcpy(line, input);
    printf("Splitting %s!\n", line);
    int i= 0, j=0, argc=0;
    char ** myargv = NULL;
    char delim[] = "/";
    
    // count how many delims we have ( or how many args we have )
    while( line[i] != NULL )
    {
    	if ( line[i] == '/' ) { ++argc; }
    	++i;
    }
    
    myargv = (char **)malloc( argc * sizeof(char *) );
    
    // Tokenize the string
	result = strtok( &line[1], delim);
    for (i = 0; i < argc; ++i)
    {
    	// allot space for the new string and add it
        myargv[i] = (char *)malloc(sizeof(result)+1);
        strcpy(myargv[i] , result);
        // terminate
        strcat(myargv[i], "\0");
        result = strtok( NULL, delim);
    }
    // make absolute sure the string is terminated
    myargv[i] = NULL;

    *c = argc;
    return myargv;
}
void inode()
{
	int c;
	char buff[1024]={0};

	lseek(fd, 1024 * blk + (128*(disp-1) ), 0);
	c = read(fd, buff, 1024);
	ip = (struct ext2_inode *)buff + 1 ;
	printf("c: %d\n",c);
    
	printf("================== Inode ===================\n");
	printf("Inode mode:\t%x\n",ip->i_mode);			/* File mode */
  	printf("Inode UID:\t%d\n", ip->i_uid);				/* Owner Uid */
  	printf("Inode Size:\t%d\n", ip->i_size);			/* Size in bytes */
	size = ip->i_size;
  	printf("Inode ID:\t%d\n", ip->i_gid);				/* Group Id */
  	printf("Inode Count:\t%d\n", ip->i_links_count);	/* Links count */
  	printf("Inode #Blk:\t%d\n",ip->i_blocks);     		/* Blocks count */
  	printf("Block 0:\t%d\n",ip->i_block[0]);  			/* Pointers to blocks */
  	printf("============================================\n");
	if ( S_ISREG( ip->i_mode ) )
	{
	  disk();
	  close(fd);
	  exit(100);
	}
}

void disk(){
	  int i,c;
	  unsigned long buf[1024]={0};
	  unsigned long block[1024]={0};
	  printf("Printing out blocks\n");
	  printf("==================== Disk =======================\n");
	  printf("size: %d\tBlocks: %d\n", ip->i_size, i = 1 + ((ip->i_size - 1) / 1024) );
	  printf("********************direct***********************\n");
	  for ( c = 0 ; c < 12; ++c)
	  {
	    if( ip->i_block[c] == 0 ) { break; }
	    printf("Block[%d] = %d\n", c, ip->i_block[c]);
	  }
	  i -= 12;
	  printf("-------------------------------------------------\n");
	  
	  if ( ip->i_block[c] != 0 )
	  {
	    printf("*******************indirect**********************\n");
	    lseek( fd, 1024* ip->i_block[12] , 0 );
	    read ( fd, buf, 1024);
	    c = 0;
	    while ( c < 256){
	      printf("%4d  ", buf[c] );
	      if( (c % 8) == 0 ) {printf("\n");}
	      --i;
	      c += 1;
	      if ( i == 0 ){ break; }
	    }
	    printf("\nremaining: %d\n", i);
	    printf("-------------------------------------------------\n");
	  }
	  
	  if ( i > 0 )
	  {
	    int j=0;
	    printf("****************Double inDirect********************\n");
	    lseek( fd, 1024 * ip->i_block[13] , 0);
	    read( fd, block, 1024);
	    while ( j < 256 )
	    {
	      lseek( fd, 1024 * block[j], 0 );
	      read( fd , buf, 1024);
	      c = 0;
	      while(c < 256)
	      {
		printf("%4d  ", buf[c] );
		if( (c % 8) == 0 ) 
		  printf("\n");
		--i;
		++c;
		if ( i == 0 ){ break; }
	      }
	      ++j;
	      if ( i == 0 ){ break; }
	    }
	    printf("\nremaining: %d\n", i);
	    printf("-------------------------------------------------\n");
	  }
	  
	  if ( i > 0 )
	  {
	    int j=0, k=0;
	    printf("****************Triple inDirect********************\n");
	    lseek( fd, 1024 * ip->i_block[14] , 0);
	    read( fd, block, 1024);
	    while( k < 256)
	    {
	      while ( j < 256 )
	      {
		lseek( fd, 1024 * (unsigned long)block[k], 0 );
		read ( fd, block, 1024);
		
		lseek( fd, 1024 * (unsigned long)block[j], 0 );
		read( fd , buf, 1024);
		c = 0;
		while(c < 256)
		{
		  printf("  %d  ", buf[c] );
		  if( (c % 8) == 0 ) 
		    printf("\n");
		  --i;
		  ++c;
		  if ( i == 0 ){ break; }
		}
		if ( i == 0 ){ break; }
	      }
	      if ( i == 0 ){ break; }
	    }
	    printf("\nremaining: %d\n", i);
	    printf("-------------------------------------------------\n");
	  }
	  
	  printf("END OF DISK READS\n");
	  printf("=================================================\n");
}

int search( char * name )
{
	// search for name string in the data blocks of this INODE
	// if found, return name's inumber
	// else      return 0
	int c;
	char buff[1024];
	struct ext2_dir_entry_2 *dp = (struct ext2_dir_entry_2 *)buff;    // access buf[] as DIR entries
	char *cp = buff;            // char pointer pointing at buf[ ]
	
	lseek(fd, 1024*ip->i_block[0] , 0 );
	c = read(fd, buff , 1024);
	
	if ( S_ISREG( ip->i_mode ) )
	{
	  printf("thats a regular file! We're done here\nExiting\n");
	  return -1;
	}
	
	char n[1024];
	c = 0; // total counter
	printf("Inode\tRec_len\tName_len\tName\n");
	while(cp < buff + 1024){
	  strncpy( n , dp->name ,dp->name_len );
	  n[dp->name_len] = '\0';
	  printf("%d\t%d\t%d\t\t%s\n", dp->inode,dp->rec_len, dp->name_len, n);
	  if (!strcmp(name, n ) )
	  {
	    printf("Found %s at %d\n", name , dp->inode);
	    return dp->inode;
	  }
	  
	  c += dp->rec_len;
	  if ( c == 1024 )
	    break;
	    
	  if(dp->inode == 0){ break;}
	  cp += dp->rec_len;         // advance cp by rlen in bytes
	  dp = cp;       // pull dp to the next record
	}
	return 0;
}

main(int argc, char *argv[ ])
{
	int c=0, i=0;
	char *line = argv[2], **name;
	
	char buff[1024];
	// Open her up! 
	fd = open( argv[1] , O_RDONLY);
	if ( fd < 0 )
		printf("Error Reading from disk!\n");
	  exit(100);

	if ( superblock() )
	{	
		printf("ITS A EXT2 fs\n");
	}else{
		printf("Its not an ext2 fs, No idea what to do exiting");
		getchar();
		exit(100);
	}

	if ( group() )
	{ 
		printf("read group gud\nInodesBeginBlock : %d\n",InodesBeginBlock ); 
	}
	
 	name = strToArray(line, &c);
 	blk = InodesBeginBlock;
	disp = 1;
 	while( name[i] != NULL ){
 		printf("\n------------Searching for name[%d] = %s-----------\n",i, name[i]);
		inode();
		c = search( name[i] );
		if ( c == 0 )
		{
		  printf("Trouble finding %s, Exiting\n", name[i]);
		  exit(100);
		}
		blk = (c-1) / ipb + InodesBeginBlock;
		disp = (c-1) % ipb;
		
		printf("blk: %d\tdisp: %d\tipb: %d\n", blk, disp, ipb);
		//printf("size: %d\n", ip->i_mode);
		++i;
	}
	inode();
	disk();
	close(fd);
}
