#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

int STORE;

struct partition {
	unsigned char drive;
	unsigned char head;
	unsigned char sector;
	unsigned char cylinder;
	unsigned char sys_type;
	unsigned char end_head;
	unsigned char end_sector;
	unsigned char end_cylinder;
	unsigned long start_sector;
	unsigned long nr_sectors;
};

main()
{
	int fd;
	struct partition *p;
	char buf[512];
	
	fd = open("vdiskImage", O_RDONLY); // Read
	if (fd != -1 )	// Open was good
	{	
		if ( 512 == read(fd , buf, 512)) //Correctly read 512 bytes
		{
			p = (struct partition *)(&buf[0x1be]); //Cast the 512 bytes to the struct
			int i;
			long end;
			for ( i=0;i<4;++i)		// Loop through the 4 partitions
			{
				printf( "\n" );
				printf( "vdiskImage%i \n", i);
				printf( "Start=%lu \n",	p->start_sector);				
				end = ( p->start_sector + p->nr_sectors -1 );				
				printf( "End=%lu \n", end);				
				printf( "Blocks=%lu \n", ((end - p->start_sector)/2) +1 ) ;
				printf( "Type=%2x \n", p->sys_type);			
				p++;
			} 			
		}
	}else
	{
		printf("Read Error");
	}
}
