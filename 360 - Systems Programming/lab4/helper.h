#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>
#define MAX 256

char ** getInput(char * line);
int cFile(char *filename);
char * findPath(char * command);
void cd(char** myargv);
void runCMD(char** myargv, char *env[]);
void ls(char* cwd);

void rFile( int sock , char *fn );
void wFile( int sock , char *fn );

int fCMD(char * input);

void runLocal(char** myargv, char* env[]);
