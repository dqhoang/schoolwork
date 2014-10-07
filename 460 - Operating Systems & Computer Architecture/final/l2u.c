#include "ucode.c"

#define BUF_SIZE 64

int main(int argc, char* argv[])
{
  int fileFrom, fileTo, byte, copy;
  STAT stats;
  char buf[64], *p;
  
  if (argc != 3)
  {
    printf("Invalid: ltu FileName Filename\n");
    exit(1);
  }
  
  fileFrom = open(argv[1], READ);
  
  stat(argv[1], &stats);
  if(stats.st_size == 0)
  {
    printf("Invalid: File %s doesn't exist\n");
    exit(1);
  }
  printf("opening %s for writing\n", argv[2]);
  creat(argv[2]);
  fileTo = open(argv[2], WRITE);
  
  printf("LTU: f%d[%s] size[%d] and f%d[%s] opened\n",fileFrom, argv[1],stats.st_size,fileTo, argv[2]);
  byte = stats.st_size;
  while(byte > 0 )
  {
    if(byte < 64){
      copy = byte;
    }else{
      copy = 64;
    }
    
    read(fileFrom, buf, copy);
    
    p = buf;
    
    while( p < buf +64)
    {
      if( *p >= 'a' && *p<= 'z')
      {
        *p -= 32;
      }
      ++p;
    }
    
    write(fileTo, buf, copy);
    
    byte -=copy;
    
  }
}
