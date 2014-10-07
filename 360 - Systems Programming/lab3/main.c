#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

char args[128] = {0};
int argc=0;
char ** myargv = NULL;
char IN[128] = {0};
char APP[128] = {0};
char OUT[128] = {0};

void getInput()
{
    int i;
    char line[128];
    char delim[] = " ";
	char *result = NULL;
    gets(line);

    if(line[0] == '\0' || line[0] == '\n')
    {
        line[0] = '\0';
        return;
    }

	result = strtok( line , delim );
	while (  result != NULL )
	{
	    if( strcmp(result, ">") == 0 )
	    {
	        // Redirect Out
            strcpy(OUT, strtok(NULL,delim));
            printf("OUT IS: %s\n", OUT);
	    }
	    else if( strcmp(result, "<") == 0 )
	    {
	        // Redirect In
            strcpy(IN, strtok(NULL,delim));
            printf("IN IS: %s\n", IN);
	    }else if( strcmp(result, ">>") == 0 )
	    {
            strcpy(APP, strtok(NULL,delim));
            printf("APPENDING TO: %s\n", APP);
	    }else if( strcmp(result, " ") == 0 )
	    {

	    }else{
            strcat(args, result);
            strcat(args, " ");
            ++argc;
        }
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
    //printf("This is Input, argc is: %d", argc);
    myargv[i] = NULL;
}

void freeMyPeople()
{
    memset(IN, 0, 128);
    memset(OUT, 0 , 128);
    memset(APP, 0 , 128);
    int i=0;
    if(myargv != NULL)
    {
        i = 0;
        while( myargv[i] != NULL )
        {
            //free(myargv[i]);
            myargv[i] = NULL;
            ++i;
        }
        //free(myargv);
        myargv = NULL;
        argc = 0;
        memset(args, 0 , strlen(args));
    }
}

int cFile(char *filename)
{
  struct stat   buffer;
  return (stat (filename, &buffer) == 0);
}

char * findPath(char * command)
{
    int i=0;
    int j=0;
    int pos;
    char * result= "";
    char current[128]= "";
    char * cmd = command;
    char * paths = getenv("PATH");
    //printf("PATHS IS : %s", paths);
    while( paths[i] != '\0')
    {
        if(paths[i] == ':')
        {
            // check current
            current[j] = '/';
            current[j+1] = '\0';
            strcat(current, cmd);
            printf("Checking %s\n", current);
            if( cFile(current) )
            {
                return current;
            }
            j = 0;
            ++i;
        }
        current[j] = paths[i];
        ++j;
        ++i;
    }
}

void cd()
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

int main(int argc, char *argv[], char *env[])
{
    int pid=0;
    int status=0;
    char * file = NULL;
    char * cmd= NULL;
    char * current = NULL;
    int i=0;

    current = getenv("PWD");
    while(1)
    {
        getcwd(current, 128);
        printf("~%s~: ", current );
        //freeMyPeople();
        getInput();
        if(myargv == NULL || myargv[0] == '\0')
        {

        }else{
        //printf("After get input argc is: %d", argc);
        cmd = myargv[0];

        if( strcmp(cmd, "cd") == 0)
        {
            cd();
        }else if ( strcmp(cmd, "exit") == 0 )
        {
            exit(EXIT_SUCCESS);
        }else
        {
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
                if( cmd[0] == '/')
                {
                    myargv[0] = strrchr(myargv[0] , '/') + 1;
                    if( cFile(cmd) )
                    {
                        file = &cmd[0];
                    }else
                    {
                        file = findPath(myargv[0]);
                    }
                }else{
                    file = findPath(cmd);
                }
                // Outputs
                printf("\nThis is child!\nI'm gonna run %s\t#ARGS: %d\n\n", file, argc);
                i = 0;

                while(myargv[i] != NULL)
                {
                    printf("arguement %d %s \n", i ,myargv[i]);
                    ++i;
                }

                // Checking for IO Redirects
                if( OUT[0] != 0 )
                {   // Redirect Out
                    strcat(current, "/");
                    strcat(current, OUT);
                    printf("OUT: %s Current: %s\n", OUT ,current);
                    close(1);   // close stdOut
                    //fclose(stdout);
                    open( current, O_WRONLY | O_CREAT, 0644 );
                }else if( APP[0] != 0)
                {
                    strcat(current, "/");
                    strcat(current, APP);
                    printf("APPENDING: %s Current: %s\n", APP ,current);
                    close(1);   // close stdOut
                    //fclose(stdout);
                    open( current, O_WRONLY| O_APPEND, 0644 );
                }

                if( IN[0] != 0 )
                {   // Redirect In
                    strcat(current, "/");
                    strcat(current, IN);
                    printf("Changing input to file: %s", IN);
                    //close(0);   // close stdin
                    fclose(stdin);
                    open( current, O_RDONLY );
                }
                execve( file , myargv , env );
            }
        }

        // Free the variables
        freeMyPeople();
        }
    }
    return 0;
}
