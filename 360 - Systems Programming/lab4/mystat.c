#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

typedef unsigned short u16;
typedef unsigned long u32;

struct stat mystat, *sp;

void main( int argc, char *argv[ ] )
{
  int r,i,j;
  char c;
  char p[9] ={0};
  char fn[128]={0};
  printf("Reading file: %s\n", argv[1]);
  r = lstat( argv[1] , &mystat);
  sp = &mystat;

  printf("FILE_TYPE\tPERMISSIONS\tUid\tSize\tCreation Time\n");
  printf("---------\t-----------\t---\t----\t-------------\n");

  if( ( sp->st_mode & 0100000) == 0100000 )
    printf("REG\t\t");
  if( ( sp->st_mode & 0040000) == 0040000 )
    printf("DIR\t\t");
  if( ( sp->st_mode & 0120000) == 0120000 )
    printf("lnk\t\t");

  i = 0;
  j = 8;
  while(i <= 8){
    if(i == 8 || i == 5 || i == 2){ c = 'r';}
    if(i == 7 || i == 4 || i == 1){ c = 'w';}
    if(i == 6 || i == 3 || i == 0){ c = 'x';}

    if ( (sp->st_mode & (1 << i) ) )
      {
	p[j] = c;
      } 
    else{
        p[j] = '-';
    }
    --j;
    ++i;
  }
  printf("%s\t", p);   
  printf("%d\t", sp->st_uid);
  printf("%d\t", sp->st_size);
  printf("%s", ctime(&sp->st_ctime));

  printf("\n");
  //
  if( ( sp->st_mode & 0040000) == 0040000 )// its a dir! do more things
    {
      DIR *dp = opendir(argv[1]);
      struct dirent *ep; 
      while( (ep = readdir(dp) ) != NULL ){
	printf("Directory: %s\n", ep->d_name);
	//	++ep;
      }
      closedir(dp);
    }
  if( ( sp->st_mode & 0120000) == 0120000 )// its a link! print the thing
    {
      r = readlink( argv[1], fn , 128);
      printf("The linked filename is: %s\n", fn);
    }


  return;
};
