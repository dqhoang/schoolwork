/************** t.c file **************************/

typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#include "ext2.h"
#define BLK 1024

u16 getblk(u16 blk, char *buf)
{
  readfd( blk/18, ((blk*2)%36)/18, ((blk*2)%36)%18, buf);
}

GD    *gp;
INODE *ip;
DIR   *dp;

int prints(char *s)
{
  while(*s)
    putc(*s++);
}

int gets(char *s)
{
  while( (*s = getc()) != '\r')
  {
    putc(*s);
    *s++;
  }
  *s = 0;
}

main()
{ 
  u16  i, iblk, disp, sino;
  u32	*d;
  char c;
  char name[64];
  char buf1[BLK], buf2[BLK];
  u32 buf3[BLK];
  
  /* read blk#2 to get group descriptor 0 */
  getblk((u16)2, buf1);
  gp = (GD *)buf1;
  iblk = (u16)gp->bg_inode_table; // typecast u32 to u16
  sino = (u16)gp->bg_inode_table;	// starting inode
  
  // Find boot directory
  name[0] = 'b';
  name[1] = 'o';
  name[2] = 'o';
  name[3] = 't';
  name[4] = 0;
  
  //inode block into buf1[ ]
  getblk( (u16)iblk, buf1);
  //ip point to INODE
  ip = (INODE *)buf1 + 1;
    
  i=0;
  while(i<2)
  {
    if(i == 1)
    {
      //prints("Boot:");
      gets(name);
      if(name[0] == 0)
      {
	name[0] = 'm';
	name[1] = 't';
	name[2] = 'x';
	name[3] = 0;
      }
    }
    getblk( (u16)ip->i_block[0], buf2);
    dp = (DIR *)buf2;
    while ( (char *)dp < &buf2[BLK])
    {
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = '\0';
      if (!strcmp(dp->name , name))
      {
	dp->name[dp->name_len] = c;
	iblk = (u16)dp->inode;
	break;
      }
      dp->name[dp->name_len] = c;
      dp = (DIR *)((char *)dp + dp->rec_len);
    }
    
    disp = ( iblk-1 ) % 8;
    iblk = ( (iblk-1) / 8 ) + sino;
    
    //inode block into buf1[ ]
    getblk( (u16)iblk, buf1);
    //ip point to INODE
    ip = (INODE *)buf1 + 1;
    ip += (disp -1);
    ++i;
  }
  /*******************************************************         
  (3). From the file's inode, find the disk blocks of the file:
      i_block[0] to i_block[11] are DIRECT blocks
      MTX kerenl has at most 64 (1KB) blocks, so no double-indirect blocks.
  *******************************************************/
  getblk( (u16)ip->i_block[12], buf2);
  getblk( (u16)ip->i_block[13], buf3);
  d = buf2;
  setes(0x1000);
  /*******************************************************
  (4). Load the blocks of /boot/mtx into memory at the segment 0x1000.
  *******************************************************/
  prints("\n\r");
  for ( i =0 ; i < 12 ; ++i)
  {
    getblk( (u16)ip->i_block[i], 0);
    putc('.');
    inces();
  }
  
  while ( (char *)d < &buf2[BLK] && *d != 0)
  {
    getblk( (u16)*d, 0);
    inces();
    putc('.');
    ++d;
  }
  /*
  while ( i=0;i<256;++i)
  {
    
    while ( (char *)d < &buf2[BLK] && *d != 0)
    {
      getblk( (u16)*d, 0);
      inces();
      ++d;
    }
  }
  /*******************************************************
  (5). Any errro condition, call error() in assembly.
  *******************************************************/
} 

