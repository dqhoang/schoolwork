/*
 * Server 
 * Written by Duy Hoang
 * Wed Oct 2
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>

struct sockaddr_in server_addr, client_addr, name_addr;
struct hostent *hp;

int sock;	// socket
int sPort;	// server Port
int id;

int IDS[] = { 12345678 , 87654321 , 11275699 };
char* USER[] = { "Joe", "Shmoe", "Duy" };
char* PASS[] = { "qwerty", "ytrewq", "yut" };

int cID( int j){
 int i;
 for (i = 0; i < 3 ; ++i){
  if (j == IDS[i] ){ 
    id = i;
    return 1;
  }
 }
 return 0;
}

int cUSER( char* user){
  
  if ( !strcmp( user, USER[id] ) ){ return 1; }
  return 0;
}

int cPass( char* pass){
  if ( !strcmp( pass, PASS[id] ) ){ return 1; }
  return 0;
}

int serverInit(char* host)
{
  printf("===== Initializing Server =====\n");
  
  hp = gethostbyname(host) ;
  if(hp == 0){
    printf("Host Unknown\n");
    exit(1);
  }
  
  printf("= Host:%s \t IP= \n", hp->h_name, inet_ntoa( *(long *)hp->h_addr));
  
  if ( (sock = socket( AF_INET, SOCK_STREAM, 0)) < 0 ){
    printf("Error calling socket\n");
    exit(1);
  }
  
  printf("= Filling Server_addr\n");
  // Initialize server address
  server_addr.sin_family = AF_INET;			// TCP/IP
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// THIS HOST IP addresss
  server_addr.sin_port = htons(sPort);				// assign port
  
  printf("= Bind socket\n");
  if ( bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ){
    printf("Bind Failed\n");
    exit(1);
  }
  
  int length = sizeof(name_addr);
  if( getsockname(sock, (struct sockaddr *)&name_addr, &length) < 0 ){
    printf("Sockname error\n");
    exit(1);
  }
  
  sPort = ntohs(name_addr.sin_port);
  printf("= Server is listening on Port=%d \n", sPort);
  printf("===== Server Init Done =====\n");
}

main (int argc, char *argv[], char *env[])
{
  char *hostname; 
  char in[512];
  uint16_t l;
  int i, r;
  
  hostname = "localhost";
  if (argc < 2)
    sPort = 0;
  else
    sscanf(argv[1], "%d", &sPort);
  
  serverInit(hostname);
  listen(sock, 5);
  while(1){
    printf("Server: Accepting New Connections\n");
    int length = sizeof(client_addr);
    int cSock = accept(sock, (struct sockaddr *)&client_addr, &length);
    if ( cSock < 0 ){printf("Server: Accept Error\n");exit(1);}
    
    printf("Server: New Connection:\n IP = \t Port = \n\n", inet_ntoa(client_addr.sin_addr.s_addr),
								ntohs(client_addr.sin_port));
    
    
    printf("Server: Sending Welcome Message\n");
    strcpy( in , "Welcome to the Jungle");
    in[strlen(in)] = '\n';
    if ( write(cSock, in, 22) < 0 ){ printf("Server: Welcome Message Failed\n"); break;}
    
    // loop for ID and User
    for (i =0; i < 3; ++i){
      memset( in, 0, 512);
      r = read( cSock, in, 256);
      printf("Server: ID from client: %s\n", in);
      sscanf(in, "%d", &id);
      memset( in , 0, 512);
      r = read( cSock, in, 256);
      printf("Server: User from client: %s\n", in);
      
      if ( cID( id ) && cUSER(in)  ){
	memset( in, 0, 512);
	strcpy( in , "Success");
	write( cSock, in , 256);
	printf("Server: Found User\n");
	break;
      }else
      {
	memset( in, 0, 512);
	strcpy( in , "Failure");
	write( cSock, in , 256);
	printf("Server: Invalid User\n");
      }
     if( i == 2){
	// Out of Tries
       close(cSock);
     }
    }
    
    // Get Passwords
    while(1){
      l=NULL;
      while(1)
      { // Force a flush and syncs the server and client
	memset(in, 0, 512);
	read(cSock, in, 512);
	printf("Server: [ %s ]\n", in);
	if( !strcmp( in, "Send Pass") )
	   {
	     printf("Server: Ready for Password\n");
	     break;
	   }
      }
      memset( in , 0 , 512);
      r = read(cSock, &l , 512 );
      printf("Server: Password length is: %d\n", ntohs(l));
      read(cSock, in, ntohs(l));
      printf("Server: Password is: %s\n", in);
      
      if( !cPass(in) ){
	memset( in , 0, 512);
	strcpy( in, "Password Incorrect");
	printf("Server: [ %s ]\n", in);
	write( cSock, in , strlen(in) );
      }else
      {
	memset( in , 0, 512);
	sprintf( in, "Congratulations %s; you just revealed the password for %d to the world!\n", USER[id], IDS[id]);
	write( cSock, in , strlen(in) );
	printf("Server: Password Matched\n");
	break;
      }
    }
    
    close(cSock);
  }
}