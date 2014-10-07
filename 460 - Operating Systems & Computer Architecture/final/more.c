#include "ucode.c"
/*
	Interactive Commands
	----------------------
 	space   - print 1 page
 	enter   - print 1 line
 	any key - print 1 char
 */

int main(int argc, char* argv[]) 
{
	STAT status;
	char line[128];
	int size, row, col;
	char c, tmp;

	int fd = open(argv[1], 0);
	if (argc != 2)
	{
		printf("usage: more filename\n");
		exit(1);
	}

	stat(argv[1], &status);

	size = status.st_size;

	printf("more: fd=%d size_remain=%d\n", fd, size);

	if (size == 0)
		exit(0);

	while(1)
	{
		// 24 rows, 80 columns
		scroll:

		for(row = 0; row < 24; ++row)
		{
			for(col = 0; col < 80; ++col)
			{
				read(fd, &c, 1);
				putc(c);

				size--;

				if (size == 0)
					exit(0);

				if (c == '\n' || c == '\r')
					break;	// end of line
			}
		}

		// get user input
		while(1) {
			c = getc();
			switch(c)
			{
				case 'q':
					close(fd);
					exit(0);

				case '\r':
					// print a line
					{
						for(col = 0; col < 80; ++col)
            {
							read(fd, &c, 1);
							putc(c);

							if (c == '\n' || c == '\r')
								break;	// end of line
						}
					}
					break;
				case ' ': goto scroll;	
				default:
					read(fd, &c, 1);
					putc(c);
			}
      size--;
      if (size == 0)
					exit(0);
		}
	}
}
