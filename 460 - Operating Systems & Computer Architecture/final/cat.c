#include "ucode.c"

int main(int argc, char* argv[])
{
  char c;
  char buf[64];
  STAT info;
	int size, left, fd, i=0;
  
  if(argc < 2)
  {
    while(1)
    {
      c = getc();
      switch(c)
      {
        case '\r': printf("\n");break;
        case '~' : exit(0);     break;
        default:  putc(c);
      }
    }
  }
  else
  {
			fd = open(argv[1], 0);
			stat(argv[1], &info);
			left = info.st_size;

			while(left > 0)
			{
				size = 64;

				if (left < 64)
					size = left;

				read(fd, buf, size);

				for(i = 0; i < size; ++i)
				{
					c = buf[i];
					switch(c)
					{
						case '\n': putc('\n');  putc('\r');	break;
						default  : putc(c);  	break;
					}
				}

				left -= size;
			}
			close(fd);
      exit(0);
  }
}
