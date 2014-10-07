#include "helper.h"

typedef unsigned short u16;
typedef unsigned long u32;

#define MAX 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;


int  sock, newsock;                  // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables


// Server initialization code:


int server_init(char *name)
{
	printf("==================== server init ======================\n");
   // get DOT name and IP address of this host


   printf("1 : get and show server host info\n");
   hp = gethostbyname(name);
   if (hp == 0){
      printf("unknown host\n");
      exit(1);
   }
   printf("    hostname=%s  IP=%s\n",
               hp->h_name,  inet_ntoa(*(long *)hp->h_addr));


   //  create a TCP socket by socket() syscall
   printf("2 : create a socket\n");
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0){
      printf("socket call failed\n");
      exit(2);
   }


   printf("3 : fill server_addr with host IP and PORT# info\n");
   // initialize the server_addr structure
   server_addr.sin_family = AF_INET;                  // for TCP/IP
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address
   server_addr.sin_port = 0;   // let kernel assign port


   printf("4 : bind socket to host info\n");
   // bind syscall: bind the socket to server_addr info
   r = bind(sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
   if (r < 0){
       printf("bind failed\n");
       exit(3);
   }


   printf("5 : find out Kernel assigned PORT# and show it\n");
   // find out socket port number (assigned by kernel)
   length = sizeof(name_addr);
   r = getsockname(sock, (struct sockaddr *)&name_addr, &length);
   if (r < 0){
      printf("get socketname error\n");
      exit(4);
   }


   // show port number
   serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
   printf("    Port=%d\n", serverPort);


   // listen at port with a max. queue of 5 (waiting clients)
   printf("5 : server is listening ....\n");
   listen(sock, 5);
   printf("===================== init done =======================\n");
}

void ls(char * cwd)
{
  char p[10] = {0}, out[MAX] = {0}, *curr=NULL, c;
  int i = 0;
  int j = 8;
  struct stat mystat, *sp;
  
  DIR *dp = opendir( cwd );
  struct dirent *ep; 
  
  write(newsock, "STARTING LS" , MAX);
  
  while( (ep = readdir(dp) ) != NULL ){
    curr = ep->d_name;
	printf("File: %s\n", curr);
	
	i =0;
	j =8;

	// File read
	r = lstat( curr , &mystat);
	sp = &mystat;
	// Permissions bits
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
	p[10] = '\0';
	bzero(out, MAX);
	sprintf( out, "%s  %d  %d\t%s\t%s",p, sp->st_uid, sp->st_size, curr, ctime(&sp->st_ctime) );
	out[strlen(out)-1] = 0;	
	// link
	if( ( sp->st_mode & 0120000) == 0120000 )
	{
		char fn[128] = {0};
		readlink(curr , fn , 128);
		strcat(out, " -> ");
		strcat(out, fn);
	}
	if( ( sp->st_mode & 0040000) == 0040000 )// its a dir! do more things
    {
    	strcat(out, " DIR ");
    }
	printf("%s\n", out);
	write(newsock, out , MAX);
  }
  closedir(dp);
}

main(int argc, char *argv[], char *env[])
{
   char *hostname;
   char line[MAX], cwd[128];
   int errno, ID;

   if (argc < 2)
      hostname = "localhost";
   else
      hostname = argv[1];

   server_init(hostname);

   getcwd(cwd, MAX);

   /*if(chroot(cwd) < 0)
   {
	printf("Root change issue:\nerrno=%d : %s\n", errno, strerror(errno));

	exit(0);
   }*/


   // Try to accept a client request
   while(1){
     printf("server: accepting new connection ....\n");

     // Try to accept a client connection as descriptor newsock
     length = sizeof(client_addr);
     newsock = accept(sock, (struct sockaddr *)&client_addr, &length);
     if (newsock < 0){
        printf("server: accept error\n");
        exit(1);
     }
     printf("server: accepted a client connection from\n");
     printf("-----------------------------------------------\n");
     printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                                        ntohs(client_addr.sin_port));
     printf("-----------------------------------------------\n");


     // Processing loop
     while(1){
        char ** input = NULL;
		n = read(newsock, line, MAX);
		if (n==0){
           printf("server: client died, server loops\n");
           close(newsock);
           break;
        }
		
      // show the line string
      printf("server: read  n=%d bytes; line=[%s]\n", n, line);
        input = getInput(line);
        pArgv(input);
		ID = fCMD(input[0]);
		
		switch ( ID ){
		case 0:	// PWD
			getcwd(cwd, MAX);
			sprintf(line, "CWD: %s", cwd);
			write(newsock, line, MAX);
			break;
		case 1:	// LS
        	if( input[1] == NULL )
        		getcwd(cwd, 128);
        	else
        		sprintf(cwd,"%s", input[1]);
        	ls(cwd); 
			break;
		case 2:	// CD
			cd(input);
        	getcwd(line, MAX);
        	printf("%s\n", line);
        	write(newsock, line, MAX);
			break;
		case 3: // mkdir
			if (mkdir(input[1], 0777) < 0){
                 sprintf(line, "errno=%d : %s", errno, strerror(errno));
              }else{ 
              	sprintf(line, "Successfully Created dir: %s\n", input[1]);
            }
            write(newsock, line, MAX);
			break;
		case 4: // rmdir
			if (rmdir(input[1]) < 0){
                 sprintf(line, "errno=%d : %s", errno, strerror(errno));
              }else{ 
              	sprintf(line, "Successfully removed dir: %s\n", input[1]);
              }
            write(newsock, line, MAX);
			break;
		case 5: // rm
			if (unlink(input[1]) < 0){
                 sprintf(line, "errno=%d : %s", errno, strerror(errno));
              }else{ 
              	sprintf(line, "Successfully deleted file: %s\n", input[1]);
              }
            write(newsock, line, MAX);
			break;
		case 6: // get
			rFile(newsock, input[1]); 
			break;
		case 7: // put
			wFile(newsock, input[1]);
			break;
		}
		
      // send the end line to client
	  sprintf(line, "END STREAM");
      n = write(newsock, line , MAX);

      //printf("server: wrote n=%d bytes; ECHO=[%s]\n", n, line);
      printf("server: ready for next request\n");
    }
 }
}
