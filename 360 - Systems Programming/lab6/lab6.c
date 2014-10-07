#include "lab6.h"
#include "showblock.c"
#include "funct.c"
#include "level2.c"

int menu()
{
  int i=0;
  printf("========================MENU========================\n");
  while ( cmdlist[i] != 0 )
  {
    printf("%d:%s \n", i ,cmdlist[i]);
    ++i;
  }
  printf("====================================================\n");
  return 1;
}

int findCmd( char * name )
{
  int i=0;
  //printf("Finding command %s\n", name);
  while( cmdlist[i] != 0)
  {
    //printf("%s\n", cmdlist[i]);
    if( !strcmp( name, cmdlist[i]) )
      return i;
    ++i;
  }
  printf("Command not found\n");
  return -1;
}

int clrMP( MINODE *mp)
{
  mp->dev = 0;
  mp->ino = 0;
  mp->refCount = 0;
  mp = NULL;
}

void mount_root()
{
  /* Get Image name here*/
  char imgName[256] = "fdimage";
  printf("Image? Enter for fdimage\n");
  fgets(imgName, 256, stdin);
  imgName[strlen(imgName)-1] = 0;
  
  if( imgName[0] == NULL )
  { 
    strcpy(imgName,"fdimage");
  }
  dev = open( imgName , O_RDWR);
  if (dev < 0)
    printf("Error reading disk\n");
  sp = superblock(dev);
  if(sp->s_magic != SUPER_MAGIC)
    {
        printf("Not a EXT2 File System!!\n"); 
        exit(100); 
    }
  INODES_PER_BLOCK = ( 1024 * (1 << sp->s_log_block_size) ) / sp->s_inode_size;
  NINODES = sp->s_inodes_count;
  NBLOCKS = sp->s_blocks_count;
    
  printf("IPB: %d\tNINODES: %d\n", INODES_PER_BLOCK, NINODES);
  gp = group(dev);
  
  MINODE *mp = iget(dev, 2); // get root inode
  
  mount[0].busy = BUSY; // we're working here!
  mount[0].dev = dev;
  strncpy( mount[0].mount_name , imgName, strlen(imgName) );
  mount[0].ninodes = sp->s_inodes_count; 
  mount[0].nblocks = sp->s_blocks_count; 
  strncpy( mount[0].name, "/" , 1);
  
  mount[0].mounted_inode = mp; 
  root = iget(dev, 2);
  
  printf("Ok Mounted dev #%d on %s as %s\n", mount[0].dev , mount[0].mount_name, mount[0].name);
}

void init()
{
  int i =0;
  // initialize arrays
  for (; i<NMINODES ; ++i)
  {  
    minode[i].refCount = 0;
  }
  
  for(i = 0; i < NMOUNT; ++i)
  {  
    mount[i].busy = FREE;
  }
  
  for(i=0 ; i <NPROC; ++i)
  {  
    proc[i].status = FREE;
  }
  
  mount_root();
  
  /* Create a proc
   * point something to it?
  pid = 0;
  uid = 0;
  cwd = "/";
  */
  
  // set up the super user
  proc[0].uid = SUPER_USER;
  proc[0].pid = SUPER_USER;
  proc[0].gid = SUPER_USER;
  proc[0].ppid = SUPER_USER;
  proc[0].cwd = root;

  cur = &proc[0];
  
}

void pMtbl()
{
  int i;
  for( i = 0 ; i < NMINODES ; ++i )
  {
    if( minode[i].dev )
      printf("\tMinode[%d]\tdev:%d\tino:%lu\tref:%d\n",i,minode[i].dev,minode[i].ino,minode[i].refCount);
    else 
      break;
  }
}

int test()
{
  char c;
  while(1){
  printf("************ TEST **************\n");
  printf("1: cd pwd ls\n");
  printf("2: make a few dir(s) and show results.\n");
  c = getchar();
    switch(c){
    case '1':  
      printf("cd to / and show contents of / directory\n");
      strcpy(pathname , "/");
      my_cd(); 
      my_pwd();    
      my_ls();
      break;          
    case '2':
       bzero(pathname, 128);
       strcpy(pathname, "/a");
       printf("making /a\n");
       mk_dir();     
       my_ls();
       bzero(pathname, 128);
       strcpy(pathname, "/a/b");
       printf("making /a/b\n");
       mk_dir();
       bzero(pathname, 128);
       strcpy(pathname,"/a");
       printf("printing /a\n");   
       my_ls();        //make dir with PATHNAMEs and show results
       bzero(pathname, 128);
       strcpy(pathname, "/a/b");
       printf("cd and print /a/b\n");
       my_cd();   
       my_pwd();         //be able to cd to a pathname, and show pwd
       bzero(pathname, 128);
       strcpy(pathname, "../../");
       printf("cd up and print\n");
       my_cd(); 
       my_pwd();     //be able to cd upward and show the results
       break;
    case '3':   
      
       strcpy(pathname,"f1");
       my_creat(); 
       my_ls();          // create a few files and show that they exist
       
       strcpy(pathname,"/a/f2");
       my_creat();
       bzero(pathname, 128);
       strcpy(pathname,"/a");   
       my_ls();       //be able to create files with pathnames
       strcpy(pathname,"f1");
       strcpy(parameter,"f2");
       my_link(); 
       my_ls();            //hard link f2 to f1
       
       bzero(pathname, 128);
       bzero(parameter, 64);
       strcpy(pathname, "f1");
       my_unlink(); 
       my_ls();             //unlink f1; f2 should still exist
       
       bzero(pathname, 128);
       strcpy(pathname, "f2");
       strcpy(parameter, "f3");
       my_symlink(); 
       my_ls();          //symlink f3 -> f2
       break;
    case '4':
       bzero(pathname, 128);
       bzero(parameter, 64);
       strcpy(pathname, "/a/b/c");
       my_rmdir(); 
       bzero(pathname, 128);
       strcpy(pathname, "/a");
       my_rmdir();      //be able to handle invalid rmdir requests
       
       bzero(pathname, 128);
       strcpy(pathname, "/a/b"); 
       my_rmdir();
        
       bzero(pathname, 128);
       strcpy(pathname, "/a");
       my_rmdir();    //rmdir and show results    
       break;
    case '5':
       bzero(pathname, 128);
       bzero(parameter, 64);
       strcpy(pathname, "0766");
       strcpy(parameter, "/f1");
       my_chmod(); 
       my_ls();          //show the chmod, touch commands
       strcpy(pathname, "/a/f2");       
       my_touch();
       strcpy(pathname, "/a/f2");
       my_ls();    //show changes in the time field 
       break;
    case '0':
    default:
      return;
    }
  }
}

int main(int argc, char *argv[ ]) 
{
  int i, c ,ret, dev; 
  char line[256], cname[64];
  view = 0;
  for ( i=0; i < argc; ++i)
  {
    if (!strcmp(argv[i], "-d"))
      view = 1;
  }
  init();
  
  menu();
  while(1){
      printf("P%d~%s~: ", cur->pid, do_pwd() );
      fgets(line, 256, stdin);
      line[strlen(line)-1] = 0;  // kill the \r char at end
      if (line[0]==0) continue;

      sscanf(line, "%s %s %64c", cname, pathname, parameter);
      c = findCmd(cname);
      if ( c>0)
      {  
        ret = cmd[c]();
        }
      else
      {
        ret = -1;
        }
      switch (ret){
	      case -1:printf("Something went wrong boss\n"); break;
	      //case 100: exit(100); break;
	      case 1:	break; 
	      default:break;
      }
      bzero(cname, 64);
      bzero(pathname, 128);
      bzero(line, 256);
      bzero(parameter, 64);
  }
} /* end main */
