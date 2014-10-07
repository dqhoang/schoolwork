/**
 *
 * Duy Hoang
 * 11275699
 * CptS 455 Networking
 * Project 2 
 *
 */

#include "readrouters.c"

#define INFINITE 64
#define TIMEOUT 10

char* router;
fd_set master, readDs; // readDs is a temp copy of the Master list of fileds
int fdmax;
struct timeval tv;

char* findSock( int inter )
{
  int i;
  for (i = 0 ; i < count ; ++i)
  {
    if ( inter == neighborSocketArray[i].socket )
    {
      return neighborSocketArray[i].neighbor;
    }
  }
  return NULL;
}

linkInfo* findLink( char* r)
{
  int i;
  for( i = 0; i < linkcount; ++i)
  {
    if( !strcmp( r, linkInfoTable[i].router) )
    {
     return &linkInfoTable[i]; 
    }
  }
  return NULL;
}

int main ( int argc, char** argv)
{
  char* path = argv[1];
  router = argv[2];
  int i, nbytes, nReady; // temp var
  char buf[1024]={0};
  
  FD_ZERO(&master);
  FD_ZERO(&readDs);
  routerInfo *router1;
  linkInfo *link;
  neighborSocket* *neigh;
  
  router1 = readrouters(path);
  link = readlinks( path , router);
  neigh = createConnections(router);
  
  addFD();
  openBase();
  tv.tv_sec = TIMEOUT;
  while(1){
    FD_ZERO(&readDs);
    memcpy(&readDs, &master, sizeof(master));
    nReady = select(fdmax+1, &readDs, NULL, NULL, &tv);
    if ( nReady == 0 ){
	broadCast();
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
    }//else{
      for(i = 0; i <= fdmax && nReady > 0; ++i) { // loop all the fds
	if (FD_ISSET(i, &readDs)) { // Info sitting on this connection
	  --nReady;
	  memset( buf , 0, 1024);
	  nbytes = recv(i, buf, sizeof( buf ), 0);
	  if( nbytes > 0 ){
	    handleMsg(buf, i);
	  }
	}
      } //end for
   // }
  }//end while
}

void resetFd()
{
  readDs = master; 
  tv.tv_sec = TIMEOUT;
  tv.tv_usec = 0;
}

void handleMsg( char* buf , int fd)
{
  char cmd;
  char *arg[3];
  char *target, *pch;
  int cost;
  int i=0;
  pch = strtok (buf," ");
  while(pch != NULL){
    arg[i] = (char*)malloc( strlen( pch ));
    strcpy(arg[i], pch);
    pch = strtok (NULL," ");
    ++i;
  }
  cmd = arg[0][0];
  switch(cmd)
  {
    case 'U':
      if( arg[1][0] == router[0]) {return;}
      target = arg[1];
      sscanf( arg[2] , "%d", &cost);
      update( target, cost , fd);
      break;
    case 'L':
      if( arg[1][0] == router[0]) {return;}
      sprintf(target, "%s", arg[1] );
      sscanf( arg[2] , "%d", &cost);
      printf("Change link!\n");
      linkCost( target , cost  );
      break;
    case 'P':
      pLinks();
      break;
    default:
      printf("Defaulted message:\n");
  }
}

/** 
 * Updates the link 
 * n destination router
 * cost cost to destination from sender ( guy at the other end of inter)
 * inter, interface the message orginated from
 */
void update( char* dest, int co , int inter)
{
  int i , cost;
  char* remote;
  
  char* hop = findSock(inter);
  linkInfo* link = findLink(hop);
  if ( link == NULL) {return;}
  
  // increment cost by the cost of the distance to the neighbor
  cost = co + link->cost;
  // find the router in the table and update it's cost
  for ( i = 0 ; i < linkcount; ++i){
    if( !strcmp(linkInfoTable[i].router ,dest) ) 
    {
	if ( cost < linkInfoTable[i].cost ){
	//printf("Router(%s) here! updating link to %s with %d from %s co:%d ctn:%d lrem:%d\n", router, dest ,cost , link->router, co, link->cost, link->remotelink);
	linkInfoTable[i].cost = cost;
	linkInfoTable[i].remotelink = inter;
	broadCast();
	}
	return;
    }
  }
  
  // add to table
  printf("Not in table! adding link to:%s at i:%d\n",dest, i);
  linkInfoTable[i].router = malloc( sizeof(dest)+1 );
  strncpy( linkInfoTable[i].router , dest , sizeof(dest)+1);
  linkInfoTable[i].cost = cost;
  linkInfoTable[i].locallink = inter;
  linkInfoTable[i].remotelink = inter;// wut
  linkcount++;
  pLinks();
}

void broadCast()
{
  int i,j ;
  char message[64];
  //printf("%s is broadcasting!\n",router);
  for ( j=0;j<count;++j){
    for ( i=0;i<linkcount;++i){
      memset(message, 0, 64);
      sprintf(message,"U %s %d", linkInfoTable[i].router, linkInfoTable[i].cost);
      send( neighborSocketArray[j].socket, message, strlen(message), 0);
    }
  }
  //printf("%s is done broadcasting!\n",router);
}

void pRouter()
{
  int i;
  printf("Router info: \n");
  for ( i = 0 ; i < routercount; ++i){
      printf("\t router: %s  ||   host: %s  ||   baseport: %d\n", routerInfoTable[i].router , routerInfoTable[i].host, routerInfoTable[i].baseport);
  }
}

void pLinks()
{
  int i;
  char* local, remote;
  printf("links:  %s\n", router);
  for ( i = 0 ; i < linkcount; ++i){
      //local = findSock(linkInfoTable[i].locallink);
      //if ( remote = findSock(linkInfoTable[i].remotelink) == NULL ){ remote = "none";}
      printf("\t router: %s  ||   cost: %d  ||   local: %d ||  remote:%s\n", linkInfoTable[i].router , linkInfoTable[i].cost, linkInfoTable[i].locallink , findSock(linkInfoTable[i].remotelink) ) ;
  }
}

void pNeigh()
{
  int i;
  printf("neighbors:\n");
  for ( i = 0 ; i < count; ++i){
      printf("\t router: %s  ||   socket: %d\n", neighborSocketArray[i].neighbor , neighborSocketArray[i].socket);
  }
}

void openBase()
{
  struct sockaddr_in sin; 
  struct hostent *hp;          
  int sockfd, i;
  int port = -1;
  for ( i = 0 ; i < routercount; ++i){
      if( !strcmp( routerInfoTable[i].router , router) )
      {
	printf("%s baseport is %d\n", router, routerInfoTable[i].baseport);
	port = routerInfoTable[i].baseport;
	break;
      }
  }
  if(port == -1){ exit(1);}
  hp = gethostbyname("localhost");
  // build address data structure of INADDR(all interfaces) paired with source port  
  bzero( (char*)&sin, sizeof(sin) );
  sin.sin_family = AF_INET;
  bcopy( hp->h_addr, (char*)&(sin.sin_addr), hp->h_length );
  sin.sin_port = htons(port);

  // Create datagram socket
  if( (sockfd = socket(PF_INET, SOCK_DGRAM, 0) ) <  0)
  {
    perror( "Error creating socket" );
    exit(1);
  }
  printf("Baseport fd is: %d\n", sockfd);
  // Bind socket to local port
  if ((bind( sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in) )) < 0 )
  {
    perror( "Error binding socket" );
    exit(1);
  }
  if ( fdmax < sockfd) { fdmax = sockfd;}
  FD_SET( sockfd, &master);
}

void addFD()
{
  int i;
  FD_ZERO(&master);
  fdmax = 0;
  for(i=0;i<count;++i){
   if( neighborSocketArray[i].socket > fdmax ){ fdmax = neighborSocketArray[i].socket; }
   FD_SET( neighborSocketArray[i].socket , &master);
  }
}

void linkCost( char* n , int cost)
{
  int i, rem=-1;
  for( i =0; i < count;++i)
  {
    if( !strcmp(neighborSocketArray[i].neighbor, n) )
    {
      rem = neighborSocketArray[i].socket;
    }
  }
  if( rem == -1){ printf("link update failed: not a neighbor\n"); return;}
  // change the link table
   for ( i = 0 ; i < linkcount; ++i){
    if( !strcmp( linkInfoTable[i].router, n) ) 
    {
      linkInfoTable[i].cost = cost;
      linkInfoTable[i].remotelink = rem;
      broadCast();
    }
   }
}

