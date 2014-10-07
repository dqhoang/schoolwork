#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node
{
	char mName[64];
	char mType;
	struct node *p;
	struct node *s;
	struct node *c;
};

struct node *root, *cwd;                      /* root and CWD pointers */
char line[128];                               /* user input line */
char command[16], pathname[64];               /* user inputs */
char dirname[64], basename[64], curr[64];               /* string holders */

void initialize()
{
	root = (struct node *)malloc( sizeof(*root) );
	strcpy(root->mName, "/");
	root->mType = 'D';
	// Pointers to NULL
	root->p = root;
	root->c = NULL;
	root->s = root;
	// current set as root
	cwd = root;
	printf("Initialized\n");
	printf("Shiny, lets be bad guys\n");
}

void menu()
{
	printf("\n");
	printf("======================  MENU  =======================\n");
	printf("mkdir rmdir ls  cd  pwd  creat  rm  save reload  exit\n");
	printf("=====================================================\n");
}
//Returns the parent of input dir
struct node* search(char c[64])
{
	int i=0,k=0,d=0;
	char curr[64];
	char delim[] = "/";
	char *result = NULL;

	// setup the work directory pointer
	struct node* wd;
	if (c[0] == '/')
	{
	  if ( strrchr(c,'/') == strchr(c,'/') )
		{
			return root;
	  }
	  wd = root;
	}else
	{
	  wd = cwd;
	  if ( strrchr(c,'/') == NULL)
	  {
		return cwd;
	  }
	}
	// Dirname has multiple forward slashes lets loop
	result = strtok(c, delim);
	while( result != NULL )
	  {
	    //printf("Searching for %s against child:%s\n",result, (wd->c)->mName);
		  if( strcmp( (wd->c)->mName, result) == 0)
		  {// Found a match in the current dir proceed down the chain
			  result = strtok(NULL,delim);
			  //printf("Found moving to child of %s", wd->mName);
			  wd = wd->c;
		  }else
		  {
			  //printf("Child wasn't good checking for siblings\n");
			  if ( (wd->c)->s == NULL){return NULL;}

			  wd = (wd->c)->s;
			  while (wd != NULL)
			  {
				  //printf("Current sibling name is:%s result is still:%s\n", wd->mName, result);
				  //printf("string cmp returned: %d\n", strcmp( wd->mName, result));
				  if( strcmp( wd->mName, result) == 0 )
				  {
					  result = strtok(NULL,delim);
					  break;
				  }else
				  {
					  if( wd->s == NULL) { return NULL;}
					  wd = wd->s;
				  }
			  }

			  //if(wd->s == NULL && wd->mName != result){return NULL;}
		  }
	}
	return wd;
}

void pathSplit()
{
	int pos=0;
	char * split;
	memset(dirname, 0 , 64);
	memset(basename, 0 , 64);
	printf("pathname=%s\n", pathname);
	split = strrchr(pathname, '/');
	if (pathname[0] == '/' || split > 0 )
	  {
	    pos = split - pathname + 1;
	    strncpy(dirname, pathname, pos);
	    strcpy(basename, split+1);
	  }
	else
	  {
	    strcpy(basename, pathname);
	  }
	printf("dirname =%s\tbasename =%s\n",dirname,basename);
}

void mkdir()
{
	char delim[] = "/";
	struct node* wd;

	printf("**** mkdir ****\n");

	pathSplit();

	//printf("dirname=%s\tbasename=%s\n",dirname,basename);
	wd = search(dirname);
	if (wd == NULL)
	  {
	    printf("Directory was bad\n");
	  }
	
	else if (wd->mType =='F') {printf("Invalid: input is a File");}
	else
	{
		printf("Search was good, Starting node work\n");

		// Built new Node

		if( wd->c == '\0' ) // Parent has no children, Base will be first child
		{
			wd->c = (struct node*)malloc(sizeof *(wd->c));
			(wd->c)->p = wd;
			wd = wd->c;
		}else
		  {
		    if( strcmp( (wd->c)->mName , basename) == 0 && (wd->c)->mType == 'D' ) 
		      { 
			printf("Directory exists; exiting\n");
			return;
		      }  
		  // cycle children till we find the right one
			struct node* parent = wd ;
			wd = (wd->c);
			while(wd->s != NULL)
			{
			  if( strcmp( (wd)->mName , basename) == 0 && wd->mType == 'F') 
			    { 
			      printf("Directory exists; exiting\n");
			      return;
			    }
			  wd = wd->s;
			}
			wd->s = (struct node *)malloc(sizeof *(wd->s));
			(wd->s)->p = parent;
			wd= wd->s;
		}
		strcpy(wd->mName, basename);
		wd->mType = 'D';
		wd->c = '\0';
		wd->s = NULL;
	  }
}

char* pwd( struct node* c )
{
  char s[128];
  memset(s,0,128);
  if(c == root)
  {
	  strcpy(s,"/");
	  return s;
  }
	strcat(s, pwd(c->p) );
	//printf("At node %s parent is:%s\n", c->mName, (c->p)->mName);
	strcat(s,c->mName);
	strcat(s,"/");
	return s;
}

void rmdir()
{
	struct node * wd;
	struct node * tmp;
	pathSplit();
	wd = search(dirname);
	if ( wd != NULL){
	if ( strcmp(basename, (wd->c)->mName) == 0 )
	{
		if ( ( wd->c )->c != NULL ){printf("Dir has children"); return; }
		tmp = (wd->c)->s;
		free(wd->c);
		wd->c = tmp;
	}else
	{
		wd = wd->c;
		while ( strcmp((wd->s)->mName, basename) != 0 )
		{
			wd = wd->s;
		}
		
		if ( ( wd->s )->c != NULL ){printf("Dir has children"); return; }
		tmp = (wd->s)->s;
		free(wd->s);
		wd->s = tmp;
	}}else{ printf("Problem finding dir");}
}

void creat()
{
	char delim[] = "/";
	struct node* wd;

	printf("**** Creat ****\n");

	pathSplit();

	printf("dirname=%s\tbasename=%s\n",dirname,basename);
	wd = search(dirname);
	if (wd == NULL)
	  {
	    printf("Directory was bad\n");
	  }
	else if (wd->mType =='F') {printf("Invalid: input is a File");}
	else
	{
		printf("Search was good, Starting node work\n");

		// Built new Node

		if( wd->c == '\0' ) // Parent has no children, Base will be first child
		  {
		  
		    if( strcmp( (wd->c)->mName , basename) == 0 && (wd->c)->mType == 'F' ) 
		      { 
			printf("Directory exists; exiting\n");
			return;
		      }
			wd->c = (struct node*)malloc(sizeof *(wd->c));
			(wd->c)->p = wd;
			wd = wd->c;
		}else
		{	// cycle children till we find the right one
			struct node* parent = wd ;
			wd = (wd->c);
			while(wd->s != NULL)
			  {
			    if( strcmp( (wd)->mName , basename) == 0 && (wd)->mType == 'F' ) 
			      { 
				printf("Directory exists; exiting\n");
				return;
			      }
				wd = wd->s;
			}
			wd->s = (struct node *)malloc(sizeof *(wd->s));
			(wd->s)->p = parent;
			wd= wd->s;
		}
		strcpy(wd->mName, basename);
		wd->mType = 'F';
		wd->c = '\0';
		wd->s = NULL;
	  }
}

void rm()
{
	struct node * wd;
	struct node * tmp;
	pathSplit();
	wd = search(dirname);
	if ( wd != NULL){
	if ( strcmp(basename, (wd->c)->mName) == 0 )
	{
		if ( ( wd->c )->mType != 'F' ){printf("Not a file"); return; }
		tmp = (wd->c)->s;
		free(wd->c);
		wd->c = tmp;
	}else
	{
		wd = wd->c;
		while ( strcmp((wd->s)->mName, basename) != 0 )
		{
			wd = wd->s;
		}
		
		if ( (wd->s)->c != NULL ){printf("Dir has children"); return; }
		tmp = (wd->s)->s;
		free(wd->s);
		wd->s = tmp;
	}}else{ printf("Problem finding dir");}
}

void write(FILE *f, struct node* n)
{
  char s[128];
  memset(s, 0 ,128);
  strcpy( s , pwd(n));
  if( s[strlen(s) - 1] == '/'){ s[strlen(s) - 1] = '\0';}
  fprintf( f , "%c %s\n", n->mType, s);
  printf( "%c %s\n", n->mType, s);
  if(n->c != NULL){ write(f,n->c);}
  if(n->s != NULL){ write(f,n->s);}
}

void save()
{
  if( root->c != NULL){
    char s[128];
    if ( pathname[0] != 0 )
      { strcpy(s, pathname);}
    else{strcpy(s, "myfile.txt");}
    FILE * file =fopen( s ,"w");
    write(file,root->c);
    fclose(file);
    printf("saved to file:%s", s);
    getchar();
  }
}

void reload()
{
  char s[128];
  char d;
  FILE * file;

  if ( pathname[0] != 0 )
    { strcpy(s, pathname);}
  else{strcpy(s, "myfile.txt");}
  
  file  = fopen( s ,"r");
  while( fscanf (file,"%c %s ", &d, pathname)  != EOF )
    {
      printf("%c %s\n", d , pathname);
      if( d == 'D' )
	{
	  mkdir();
	}else
	{
	  creat();
	}

    }
}

void cd()
{
  if(pathname[1] == '.' && pathname[0] == '.')
    {
      cwd = cwd->p;
    }
  if( strlen(pathname) == 0 )
    {
      printf("No path given; Beam me up Scotty\n");
      cwd = root;
    }
  else if(pathname[0] != '/' && strrchr(pathname, '/') == NULL)
    { 
      printf("Search current directory for name");
      if( strcmp( (cwd->c)->mName , pathname) == 0 )
	{ 
	  cwd = cwd->c; 
	}else
	{
	  struct node* wd = cwd->c;
	  while( wd->s != NULL)
	    {
	      if ( strcmp( (wd->s)->mName, pathname) == 0)
		{ cwd = wd->s; break;}else{wd = wd->s;}
	    }
	}
    }
  else
    {
      struct node* wd;
      wd = search(pathname);
      if ( wd != NULL)
	{
	  cwd = wd;
	}
    }
}


void ls()
{
	struct node *tmp;
	if (pathname == NULL){ tmp = cwd;}
	else
	{ tmp = search(pathname); }

	if(tmp->c != NULL)
	{
		tmp = tmp->c;
		printf("%c\t%s\n",tmp->mType, tmp->mName);
		while(tmp->s !=NULL)
		{
			tmp = tmp->s;
			printf("%c\t%s\n",tmp->mType, tmp->mName);
		}
	}
}

void sSplit()
{
	char delim[] = " ";
	char *result = NULL;
	result = strtok( line , delim );
	strcpy(command,result);
	result = strtok(NULL,delim);
	while (  result != NULL)
	{
		strcat(pathname,result);
		result = strtok(NULL,delim);
	}
	//printf("Split done\nPathname:%s", pathname);
}
//void (*run_cmd) = { mkdir, rmdir, ls , cd , pwd , creat , rm , save, reload, exit };
int findCommand(char cmd[16])
{
  char * options[] = {"mkdir", "rmdir", "ls" , "cd" , "pwd" , "creat" , "rm" , "save", "reload","menu", "exit" };
  int i=0;
  for(i=0;i<=10;++i)
    {
      //printf("Comparing %s to %s\n", options[i], cmd);
      if (strcmp(options[i], cmd) == 0 ){ return i; }
    }
  return -1;
}

main()
{
	int ID =0;
	initialize();      /* initialize the / DIR of the tree */
	menu();
	while(1)
	{
		memset(curr, 0 , 64);
		strcpy(curr, pwd(cwd)); 
		printf("\n%s-", curr); 
		gets(line);
		sSplit();
	    //read a line containting  command [pathname]; // [ ] means optional
	    ID = findCommand(command);
	    switch(ID){
	    case 0 : mkdir(); break;
	    case 1: rmdir(); break;
	    case 2: ls(); break;
	    case 3: cd(); break;
	    case 4: 
			memset(curr, 0, 64);
			strcpy(curr, pwd(cwd)); 
			printf("%s", curr); 
			break;
	    case 5: creat(); break;
	    case 6: rm(); break;
	    case 7: save(); break;
	    case 8: reload(); break;
	    case 9: menu(); break;
	    case 10: save(); exit(EXIT_SUCCESS); break;
	    default:
	      printf("Invalid"); break;
	  }
		memset(pathname, 0, 64);
		memset(dirname, 0, 64);
		memset(line, 0, 128);
	}
}
