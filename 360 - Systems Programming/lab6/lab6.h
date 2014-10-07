#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <sys/stat.h>

// Default dir and regulsr file modes
#define DIR_MODE          0040777 
#define FILE_MODE         0100644
#define LINK_MODE         0120777
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define BUSY              1
#define KILLED            2

// Table sizes
#define NMINODES         100
#define NMOUNT            10
#define NPROC             10
#define NFD               10
#define NOFT             100 

#define SUPERBLOCK	1
#define GDBLOCK		2
int BBITMAP;
int IBITMAP;
int INODEBLOCK;
int NINODES;
int NBLOCKS;
#define ROOT_INODE     2

static int BLOCK = 1024;
int BITS_PER_BLOCK;
int INODES_PER_BLOCK;

int dev, view;
char pathname[128]="", parameter[64]="";

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

GD    *gp;
SUPER *sp;
INODE *ip;      
DIR   *dp; 

// Open File Table
typedef struct Oft{
  int   mode;
  int   refCount;
  struct Minode *inodeptr;
  long  offset;
  char name[128];
} OFT;

// PROC structure
typedef struct Proc{
  int   uid;
  int   pid;
  int   gid;
  int   ppid;
  struct Proc *parent;
  int   status;

  struct Minode *cwd;
  OFT   *fd[NFD];
} PROC;
      
// In-memory inodes structure
typedef struct Minode{		
  INODE    INODE;               // disk inode
  ushort   dev;
  unsigned long ino;
  ushort   refCount;
  ushort   dirty;
  ushort   mounted;
  struct Mount *mountptr;
} MINODE;

// Mount Table structure
typedef struct Mount{
        int    ninodes;
        int    nblocks;
        int    dev, busy;   
        struct Minode *mounted_inode;
        char   name[256]; 
        char   mount_name[64];
} MOUNT;

MINODE minode[NMINODES];	// minode array
OFT oft[NOFT];                  
PROC proc[NPROC]; 
MOUNT mount[NMOUNT];
MINODE *root;	// points to minode 0 aka "/"
PROC *cur;

// Disk stuff
//int superblock();
SUPER* superbloc(int fd);
GD* group();
char ** strToArray( char * input , int *c);
//void inode();
void disk();
//int search( char* name);

// System functions
void mount_root();
void init();
void iput( MINODE *mp );
MINODE* iget ( int dev, int inumber );
char *do_pwd();
int my_mkdir(MINODE* pip, char* child);
char *findmyname(MINODE *mp, unsigned long myino);
long do_lseek( int fd, long position);

// User functions


int menu();	// Done
int quit();	// Done
int test(); 	// started **scraped**
/* Level 1 */
int mk_dir();	// Done
int my_rmdir();	// Done
int my_ls();	// Done
int my_cd();	// Done
int my_pwd();	// Done
int my_creat();	// Done
int my_link();	// Done
int my_unlink(); 	// Done
int my_symlink();	// Done
int my_stat();	// done
int my_chmod();	// Done
int my_chown(); // Done
int my_chgrp(); // Done
int my_touch();	// Done
/* Level 2 */
int my_open(); // coded?
int my_close(); // coded?
int my_read(); // coded?
int my_write(); // nope
int my_pfd(); // coded?
int my_lseek(); // coded?
int my_cat(); // nope
int my_cp(); // started quit
int my_mv(); // coded? bug
/* Level 3 */
int my_mount();
int my_umount();
/* File permission checking */

int findCmd(char *name);

char *cmdlist[35] = { 	"menu", 
			"quit",
			"mkdir" , 
			"rmdir", 
			"ls", 
			"cd", 
			"pwd",	
			"creat", 
			"link",  
			"unlink", 
			"symlink",
			"stat",  
			"chmod", 
			"chown",
			"chgrp",
			"touch",
			"mv",
			"open",
			"pfd",
			"close",
			"lseek",
			"read",
		//	"test",
			0 };
               
int (*cmd[35])() = { 	menu , 
			quit ,
			mk_dir, 
			my_rmdir, 
			my_ls, 
			my_cd, 
			my_pwd, 
			my_creat, 
			my_link,  
			my_unlink, 
			my_symlink,
			my_stat,  
			my_chmod,
			my_chown,
			my_chgrp,
			my_touch,
			my_mv,
			my_open,
			my_pfd,
			my_close,
			my_lseek,
			my_read,
		//	test, 
			NULL};
