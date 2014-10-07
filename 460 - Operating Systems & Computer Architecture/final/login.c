#include "ucode.c"
char *tty;
int stdin, stdout, stderr;
char shell[64];

int verify( char* username, char* password)
{
  int i, gid, uid, remain;
  char* tokens[20];
  char name[64], home[64], buf[1024];
  STAT s;
  int passfd = open("/etc/passwd", READ);
  stat("/etc/passwd", &s);
  remain = s.st_size;
  while(1)
		{
			for (i = 0; i < 1024 && remain > 0; i++)
			{
				read(passfd, &buf[i], 1);
				remain--;

				if (buf[i] == '\n') {
					buf[i+1] = '\0';
					break;
				}
			}
      i=0;
      tokens[0] = strtok(buf, ":");
      while(tokens[i++] != 0)
      {
        tokens[i] = strtok(0,":");
        //printf("%s:", tokens[i]);
      }
//6. if (user account valid){
//     setuid to user uid.
//     chdir to user HOME directory.
//     exec to the program in users's account
//   }
			if((!strcmp(username, tokens[0]) && !strcmp(password, tokens[1])))
			{
				gid = atoi(tokens[2]);
				uid = atoi(tokens[3]);
				strcpy(name, tokens[4]);
				strcpy(home, tokens[5]);
				strcpy(shell, tokens[6]);
				//printf("%s %s %d %d %s %s %s \n", username, password, gid, uid, name, home, shell);
				chdir(home);
        close (passfd);
				return 1;
			}else
      {
        //printf("psst! user was %s  pw %s \n", tokens[0], tokens[1]);
      }
			if (remain == 0)
				break;
		}
		close(passfd);
    return(0);
}

main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
  char username[64], password[64];
  char *tty =  argv[1];
  int len;

// 1. 
  close(0); close(1); close(2); // login process may run on different terms

// 2. // open its own tty (passed in by INIT) as stdin, stdout, stderr
  stdin = open(tty, 0);
  stdout = open(tty, 1);
  stderr = open(tty, 2);
// 3. 
  settty(tty);   // store tty string in PROC.tty[] for putc()

  // NOW we can use printf, which calls putc() to our tty
  printf("SkyLogin : open %s as stdin, stdout, stderr\n", tty);

  signal(2,1);  // ignore Control-C interrupts so that 
                // Control-C KILLs other procs on this tty but not the main sh

  while(1){
//1. show login:           to stdout
//2. read user name        from stdin
    printf("Skynet Login: "); gets(username);
//3. show passwd:
//4. read user passwd
    printf("code: "); 
    len = getline(password);
    password[len-1] = '\0';
//5. verify user name and passwd from /etc/passwd file
    if( verify( username, password) ) 
    {
      exec("/bin/sh");
    }
    
    printf("login failed, try again\n");
  }
}
