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
{/*
  while( (*s = getc()) != '\r')
  {
    putc(*s);
    *s++;
  }
  *s = 0;
  */
}

int find(char * name, int iblk, int sino)
{
  char c;
  char buf2[BLK];
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
  
  //inode block into buf1[ ]
  getblk( (u16)(( (iblk-1) / 8 ) + sino), buf2);
  //ip point to INODE
  ip = (INODE *)buf2 + 1;
  ip += ((( iblk-1 ) % 8) -1); 
}

main()
{ 
  u16  i, iblk, sino;
  u32	*d, *j, buf3[BLK];
  
  char *name;
  char buf1[BLK], buf2[BLK];
  
  //printSplash();
  
  /* read blk#2 to get group descriptor 0 */
  getblk((u16)2, buf1);
  gp = (GD *)buf1;
  iblk = (u16)gp->bg_inode_table; // typecast u32 to u16
  sino = (u16)gp->bg_inode_table;	// starting inode
  
  // Find boot directory
  //name = "boot";
  
  //inode block into buf1[ ]
  getblk( (u16)iblk, buf1);
  //ip point to INODE
  ip = (INODE *)buf1 + 1;
  
  find("boot", iblk, sino);
  find("mtx", iblk, sino);
  //name = "mtx";
  /*******************************************************         
  (3). From the file's inode, find the disk blocks of the file:
      i_block[0] to i_block[11] are DIRECT blocks
      MTX kerenl has at most 64 (1KB) blocks, so no double-indirect blocks.
  *******************************************************/
  getblk( (u16)ip->i_block[12], buf2);
  getblk( (u16)ip->i_block[13], buf3);
  d = buf2;
  j = buf3;
  setes(0x1000);
  /*******************************************************
  (4). Load the blocks of /boot/mtx into memory at the segment 0x1000.
  *******************************************************/
  for ( i =0 ; i < 12 ; ++i)
  {
    iblk = (u16)ip->i_block[i];
    if ( iblk == 0){return;}
    getblk( iblk, 0);
    inces();
    putc('.');
  }
  
  for( i =0; i<256; ++i)
  {
    if( *d == 0){ return;}
    getblk( (u16)*d++, 0);
    inces();
    putc('.');
  }
  putc('\r');
  putc('\n');
  getc();
  /*******************************************************
  (5). Any errro condition, call error() in assembly.
  *******************************************************/
} 

