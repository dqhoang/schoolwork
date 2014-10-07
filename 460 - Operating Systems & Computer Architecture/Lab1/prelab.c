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

main()
{ 
  u16  i, iblk;
  char c;
  
  char buf1[BLK], buf2[BLK];

  prints("booter start\n\r");  

  /* read blk#2 to get group descriptor 0 */
  getblk((u16)2, buf1);
  gp = (GD *)buf1;
  iblk = (u16)gp->bg_inode_table; // typecast u32 to u16

  prints("inode_block="); putc(iblk+'0'); getc();
  /******** write C code to do these: ********************
  (1).read first inode block into buf1[ ]
  *******************************************************/
  getblk( (u16)iblk, buf1);
  /*******************************************************
  (2).let ip point to root INODE (inode #2)
  *******************************************************/
  ip = (INODE *)buf1 +1;
  /*******************************************************
  (3).For each DIRECT block of / do:
          read data block into buf2[ ];
          step through the data block to print the names of the dir entries 
  *******************************************************/
  getblk( (u16)ip->i_block[0], buf2);
  dp = (DIR *)buf2;
  prints("\n\r***************************");
  prints("\n\rDirectories\n\r");
  while ( (char *)dp < &buf2[BLK]){
  // Print out the name
    c = dp->name[dp->name_len];
    dp->name[dp->name_len] = '\0';
    prints("    ");
    prints( dp->name );
    prints("\n\r");
    dp->name[dp->name_len] = c;
    
    dp = (DIR *)((char *)dp + dp->rec_len);
  }
  /******************************************************
  (4).prints("\n\rAll done\n\r");
  *******************************************************/
  prints("***************************");
  prints("\n\rAll Done\n\r");
} 

/*
main()
{
   char name[64];
   while(1){
     prints("What's your name? ");
     gets(name);
     if (name[0]==0)
        break;
     prints("Welcome "); prints(name); prints("\n\r");
   }
   prints("return to assembly and hang\n\r");
}

*/