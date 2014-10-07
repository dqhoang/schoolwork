#include "ucode.c"

char *shCmd[]={"?", "help", "pwd", "cd", "logout", 0};

int find( char* name)
{
  int i=0;
  
  char *cur = shCmd[0];
  while(cur)
  {
    if ( !strcmp(cur, name) )
    {
      return i;
    }
    ++i;
    cur = shCmd[i];
  }
  return -1;
}


char** tokenize(char* line, char delim)
{
  int i=0, status, pid;
  char* tok[20];
  
  tok[i] = strtok(line, delim);
  while( tok[i] != 0 )
  {
    tok[i++] = strtok(0, delim);
  }
  return tok;
}


int menu()
{
  printf(" __                                                     __ \n");
  printf("| _|              *******Menu******                    |_ |\n");
  printf("| |                                                     | |\n");
  printf("| | ?  help  pwd  cd   logout ltu   cat  cp             | |\n");
         //   0  1     2    3    4      5     6    7     8 
  printf("| |                                                     | |\n");
  printf("|__|                                                   |__|\n");
}

main(int argc,char* argv)
{
  char cmd[64], *cmds[20], cwd[64], tmp[64];  
  int r, i, status, pipes[2];
  int stdin, stdout, stderr;
  STAT s;
  printf("Welcome to Skynet\n");
  menu();
  while(1)
  {
    getcwd(cwd);
		printf("%s: ", cwd);
		gets(cmd);
		if (cmd[0] == 0)
			continue;
      // tokenize command
    i=0;
    strcpy( tmp, cmd);
    cmds[0] = strtok(tmp, " ");
    while(cmds[i++] != 0)
    {
      cmds[i] = strtok(0," ");
        //printf("%s:", tokens[i]);
    }
		
		r = find(cmds[0]);
		switch(r)
		{
			case 0:
			case 1:
				menu();
				continue;

			case 2:
				printf("%s\n", cwd);
				continue;

			case 3:
				chdir(cmds[1]); 
				continue;

			case 4:
				exit(0);

			default:
				break;
		}
    //printf("Not a shell command\n");
    // Lets call something
    if(fork())
    {
      //printf("Parents nap while kids play\n");
      pid = wait(&status);
    }else
    {
      // kids get to play for a while
      
      // Find >> > < and |
      for (i = 0; cmds[i] != 0; ++i)
      {
        switch( cmds[i][0])
        {
          case '|': 
            pipe(pipes);
            if( fork() )
            {
              cmds[i+1] = '\0';
              close(pipes[0]);
              close(1);
              dup2(pipes[1], 1);
            } else {
              close(pipes[1]);
              close(0);
              dup2(pipes[0], 0 );
            }
            break;
          case '>':
            // close std out
            // create and open file for output
            close(0);
            creat(cmds[i+1]);
            printf("cmds[%s]\n", cmds[i]);
            if( cmds[i][1] == '>' )
            {
              // append
              printf("Open %s append\n", cmds[i]);
              open(cmds[i],  O_APPEND);
            }else{
              open(cmds[i], O_WRONLY);
            }
            break;
          case '<':
            stat(cmds[i+1], &s);
            if( s.st_size == 0)
            {
              printf("Error: %s doesn't exist\n", cmds[i+1]);
              exit(0);
            }
            close(1);
            open(cmds[++i], 0);
            break;
        }
      }
      //printf("exec(%s)\n", cmd);
      exec(cmd);
      printf("Cmd Not found\n");
      exit(1);
    }
    
  }
}
