// make lcat
// make sure stuff runs locally if l...




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include "helper.h"

#define MAX 256

struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

char ** getInput( char* line )
{
	char *result = NULL;
    printf("Splitting %s!\n", line);
    int i= 0, argc=0;
    char ** myargv = NULL;
    char delim[] = " ", args[128] = {0};
	

	result = strtok( line , delim );
    while (  result != NULL )
    {
        strcat(args, result);
        strcat(args, delim);
        ++argc;

        result = strtok(NULL,delim);
    }
    myargv = (char **)malloc(argc * sizeof(char *));
    result = strtok(args, delim);
    i = 0;
    for (; i < argc; ++i)
    {
        myargv[i] = (char *)malloc(sizeof(result)+1);
        strcpy(myargv[i] , result);
        strcat(myargv[i], "\0");
        result = strtok(NULL, delim);
    }
    myargv[i] = NULL;
    return myargv;
}

void rFile( int sock , char *fn )
{
	struct stat fileStat;
	int fd=0, n, count =0, size=0;
	char line[MAX];
    write(sock, "START", MAX);
	// Stat file
	if(lstat( fn ,&fileStat) < 0)    
    {    
    	write( sock, "BAD" , MAX ); 
    	return;
    }   
    // Send bad size or If directory
    if ( !(fileStat.st_size > 0 ) || (S_ISDIR(fileStat.st_mode)) )
    {
    	write( sock, "BAD" , MAX );
    	return;
    }	
    // send File size to client
    size= fileStat.st_size;
    sprintf(line, "%d", size);
    //printf("Sending %d\n", );
	write(sock, line , MAX);
	
	fd = open( fn , O_RDONLY );
	while( count < size )
	{
		if ( (count+MAX) > size){ 
			n = size - count;
		}else{ n = MAX;}  
		n = read(fd, line, n);
		printf("Writing %d bytes to socket: %s\n", size ,line);
		write( sock, line, n);
		count += n;
	}
	close(fd);
	write( sock, "END WRITE" , MAX);   	
}

int fCMD(char * input)
{
	char* options[9] = { "pwd", "ls", "cd", "mkdir" , "rmdir", "rm", "get", "put", "cat"};
	int i;
	for ( i=0; i < 9; ++i)
	{
		if( !strcmp( options[i], input) ){ return i;}
	} 
	return -1;
}

void wFile( int sock , char *fn )
{
	int fd = 0, size = 0, count = 0, n=0;
	char line[MAX] = {0};
	// Get either size or BAD
	while( strcmp(line, "START") ){
		read( sock, line, MAX);
	}
	read( sock, line, MAX);
	printf("Bad or size: %s   \n", line);
	
	if ( !strcmp( line, "BAD") )
	{
		printf("ERROR");
		return;
	}
	
	size = atoi(line);
	
	fd = open( fn , O_WRONLY | O_CREAT , 0644);
	if( fd >0 ){
		printf("File opened %d", size);
		while( count < size )
		{
			if ( (count+MAX) > size)
			{ 
				n = size - count;
			}
			else
			{ n = MAX;} 
 
			n = read( sock, line, n);
			count += n;
			printf("Writing to file: %s\n", line);
			write( fd, line, n);
		}
		close(fd);
	}else{ write(sock, "END WRITE", MAX);}
	read(sock , line, MAX);
} 

void pArgv(char** myargv)
{
    int i = 0;
    while(myargv[i] != NULL)
    {
        printf("Arg#%d : %s\n", i , myargv[i] );
        ++i;
    }
}

int cFile(char *filename)
{
  struct stat   buffer;
  return (stat (filename, &buffer) == 0);
}

char * findPath(char * cmd)
{
    char * result= NULL;
    char current[128]= {0};
    char * paths = getenv("PATH");
    result = strtok( paths , ":" );
    while( result != NULL )
    {
        bzero(current, 128);
        strcpy(current, result);
        strcat(current, "/");
        strcat(current, cmd);
        printf("checking path! %s\n", current);
        if(cFile(current))
        {
            return(current);
        }
        result = strtok(NULL, ":");
    }
    return NULL;
}

void cd(char** myargv)
{
    printf("Changing dir\n");
    if ( myargv[1] != NULL)
    {
        if( !chdir( myargv[1]) ){printf("Change success\n");}
        else{printf("change was bad\n");}
    }
    else
    {
        puts("Going Home Boss");
        char * home;
        home = getenv("HOME");
        chdir(home);
    }
}

void lcat(char* fn)
{
	struct stat fileStat;
	int fd=0, n, count =0, size=0;
	char line[MAX];
	
	if ( lstat(fn, &fileStat) < 0 )
		return;

	printf("---FILE START---\n");
   
        // If directory
    	if (S_ISDIR(fileStat.st_mode))
    	{
    		printf("You can't read a directory!\n");
    		return;
    	}

	size= fileStat.st_size;
	fd = open( fn , O_RDONLY );

	while( count < size )
	{
		if ( (count+MAX) > size)
		{ 
			n = size - count;
		}
		else
		{ n = MAX;} 
 
		n = read(fd, line, n);
		printf("%s", line);
		count += n;
	}
	close(fd);
	printf("---FILE END---\n"); 
}

void runCMD(char** myargv, char *env[])
{
    int pid=0;
    int status=0;
    char * file = NULL;
    char * current = NULL;
    char * cmd = NULL;
    int i=0;
        if(myargv == NULL || myargv[0] == '\0')
        {
            return;
        }else{
            cmd = myargv[0];
                // Creating a child
            pid = fork();

            if( pid != 0) // parent
            {
                pid = wait(&status);
                printf("\nDEAD CHILD=%d, HOW=%d\n\n", pid, status);
            }
            else
            {
            // Searching for command
                //pArgv(myargv);
                file = findPath(cmd);
                // Outputs
                printf("\nThis is child!\nI'm gonna run %s\n\n", file);
                i = 0;
                while(myargv[i] != NULL)
                {
                    printf("arguement %d %s \n", i ,myargv[i]);
                    ++i;
                }
                execve( file , myargv , env );
            }
        }
}

void runLocal(char** myargv, char* env[])
{
	int ID = 0, i = 0, errno;
	char line[MAX], cwd[128];
	char* cmd = NULL;
	char** input = myargv;

	if(myargv == NULL || myargv[0] == '\0')
	{
		return;
	}
	else
	{
		cmd = myargv[0];

		pArgv(myargv);

		//make switch to identify lls, lcat, lpwd, lmkdir, lrmdir, lrm
		ID = fCMD(myargv[0]);
		switch(ID)
		{
		case 0: //pwd
			getcwd(cwd, MAX);
			printf("Cwd: %s\n", cwd);
			
			break;

		case 1: //ls
				if(myargv[1] == NULL){
					getcwd(cwd, 128);
				}
				else{
					sprintf(cwd, "%s", myargv[1]);
					printf("cwd: %s\n", myargv[1]);
				}
				ls(cwd);
			break;

		case 2: //cd
			cd(input);
			getcwd(line, MAX);
			printf("%s\n", line);
			break;

		case 3: //mkdir
			if(mkdir(input[1], 0777) < 0)
			{
				printf("errno= %d : %s\n", errno, strerror(errno));
			}
			else
			{
				printf("Successfully created directory: %s\n", input[1]);
			}
			break;

		case 4: //rmdir
			if( rmdir( input[1]) < 0 )
			{
				printf("errno=%d : %s\n", errno, strerror(errno));
			}
			else
			{
				printf("Successfully removed directory: %s\n", input[1]);
			}
			break;

		case 5: //rm
			if(unlink(input[1]) < 0)
			{
				printf("errno=%d : %s\n", errno, strerror(errno));
			}
			else
			{
				printf("Successfully removed file: %s\n", input[1]);
			}
			break;		

		case 8: //cat
			lcat(input[1]);
			break;
		}
	}
}
