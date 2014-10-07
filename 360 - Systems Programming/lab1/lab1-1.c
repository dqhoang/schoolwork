#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

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
			for ( i=0;i<4;++i)		// Loop through the 4 partitions
			{
				printf( "\n" );
				printf( "Partition %i \n", i);
				printf( "Drive=%x \n", 	p->drive);
				printf( "Head=%x \n",	p->head);
				printf( "sector=%x \n", p->sector);
				printf( "Cylinder=%x \n",	p->cylinder);
				printf( "Type=%x \n", p->sys_type);
				printf( "End Header=%x \n",	p->end_head);
				printf( "End Sector=%x \n",	p->end_sector);
				printf( "End Cylinder=%x \n",	p->end_cylinder);
				printf( "Start Sector=%lu \n",	p->start_sector);
				printf( "NR sectors=%lu \n",	p->nr_sectors);

				p++;
			}		
		}
	}else
	{
		printf("Read Error");
	}
}
