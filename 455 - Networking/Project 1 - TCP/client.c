/*
 * Client
 * Written By Duy Hoang
 * Wed Oct 2
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

struct hostent *hp;
struct sockaddr_in server_addr;

int sock;
int SERVER_IP, SERVER_PORT;

int clientInit(char *argv[])
{
  printf("===== Client Init =====\n");
  if( (hp = gethostbyname(argv[1]) ) == 0 ){printf("Unknown Host\n");exit(1);}
  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);
  printf("= Setting up info for connection\n");
  printf("= Server IP: %d\t Port:%d\n", SERVER_IP, SERVER_PORT);
  
  if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){printf("Socket Error\n");exit(1);}
  printf("= setting up server_addr\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);
  
  printf("= Connecting to server ... ");
  if ( connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("\tFailed\n");
    exit(1);
  }else {printf("Successfull\n");}
  
  //printf("= ---Connection Established As Follows ---\n");
  //printf("= - Host: %s   IP:%s    PORT:%d ---\n",hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("===== Init Done =====\n");
}

main(int argc, char *argv[], char *env[])
{
  int r, id, i;
  char line[512];
  uint16_t l;
  
  // Setup the Client to send messages
  clientInit(argv);
  
  // Read Welcome Message
  r = read( sock, line, 256);
  if (r < 0 ){printf("Reading welcome message bad");}
  line[22] = '\0';
  printf("Client: %s \n", line);
  if (!strcmp(line, "Welcome to the Jungle")){
    printf("Thats not what we were supposed to read... Exiting\n");
    exit(1);
  }
  i = 0;
  // Send the ID and User
  do{
    do{
      memset(line, 0, 512);
      printf("Please Enter 8 digit ID: ");
      scanf("%s", &line);
    }while ( strlen(line) != 8);
    
    r = write(sock, line , strlen(line) );
    if (r < 0 ){printf("Client: Error Sending ID\n");break;}
    
    do{
      memset(line, 0, 512);
      printf("Please Enter UserName: ");
      scanf("%s", &line);
    }while ( strlen(line) >= 20);
    
    r = write(sock, line , strlen(line) );
    if (r < 0 ){
      printf("Client: Error Sending User\n");
      break;
    }
    
    memset( line, 0, 512);
    r = read(sock, line, 256);
    printf("Client: [ %s ]\n", line);
    
  }while ( strcmp( line, "Success") );
  
  i=0;
  do{
    ++i;
    l =0;
    if ( i == 4 )
    {
      break;
    }
    // Preps the server for password and syncs the client to the server
    sprintf( line, "%s", "Send Pass");
    printf("Client: [ %s ]\n", line);
    if( write( sock, line, strlen(line) ) < 0 ){ printf("Socket Closed, exiting\n"); exit(0);}
    
    memset(line, 0, 512);
    printf("Please Enter Password: ");
    scanf("%s", &line);
    
    l = htons( strlen(line) );
    r = write(sock, &l , sizeof(uint16_t) );
    
    line[strlen(line)] = '\n';
    r = write(sock, line , strlen(line) );
    
    read( sock, line, 512);
    printf("Client: [ %s ]\n", line);
  }while (  !strcmp( line, "Password Incorrect") );
  
  close(sock);
}