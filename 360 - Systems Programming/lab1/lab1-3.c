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

main( )
{
	int fd;
	struct partition *p;
	char buf[512];

	fd = open("vdiskImage", O_RDONLY); // Read
	if ( fd != -1 )
	{	
		if ( 512 == read(fd , buf, 512) ) //Correctly read 512 bytes
		{
			p = (struct partition *)(&buf[0x1be]); //Cast the 512 bytes to the struct
			int i;
			long end, estart = 0;
			for ( i=0;i<4;++i)		// Loop through the 4 partitions
			{
				if ( estart == 0 ){	// First time through table
					printf( "\n" );
					printf( "vdiskImage%i \n", i);
					printf( "Start=%lu \n", p->start_sector);				
					end = ( p->start_sector + p->nr_sectors -1 );				
					printf( "End=%lu \n", end);				
					printf( "Blocks=%lu \n", ((end - p->start_sector)/2) +1  );
					printf( "Type=%2x \n", p->sys_type);
					if ( p->sys_type == 5)
					{	
						printf("-------------------------------");
						estart = p->start_sector;
						i=0;		// Reset the index for new table

						/* move the pointer */
						lseek( fd , (long)(estart)*512, 0 );
						read( fd , buf, 512);
						p = (struct partition *)(&buf[0x1be]);	
					}
					else	// don't increment p if it found an extend
						p++;
				}else	// estart isn't 0, going into extend mode
					// extend mode: must manually move pointer
				{	/* New print statements that have the estart offset */
					printf( "\n" );
					printf( "vdiskImage%i \n", i);
					printf( "Start=%lu \n" , estart + p->start_sector);				
					end = ( p->start_sector + p->nr_sectors -1 );				
					printf( "End=%lu \n", estart+end);				
					printf( "Blocks=%lu \n", ((end - p->start_sector)/2) +1  );
					printf( "Type=%2x \n", p->sys_type);
					/* move the offset past the partition */
					estart = p->nr_sectors + p->start_sector + estart;					
					/* Grab new P from offset */
					lseek( fd , (long)(estart)*512, 0 );
					read( fd , buf, 512);
					p = (struct partition *)(&buf[0x1be]);		
				}
				// Ok we ran outta table, we're done so break
				if( p->sys_type == 0){break;}	
			} 					
		}
	}
	else{ printf("READ ERROR"); }
}
