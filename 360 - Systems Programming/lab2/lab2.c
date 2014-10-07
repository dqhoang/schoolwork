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

struct node *root, *cwd, *wd;                      /* root and CWD pointers */
char line[128];                               /* user input line */
char command[16], pathname[64];               /* user inputs */
char dirname[64], basename[64];               /* string holders */

void initialize()
{
	root = malloc( sizeof(*root) );	
	root->mName[0] = '/';
	root->mType = 'D';
	// Pointers to NULL
	root->p = root;
	root->c = NULL;
	root->s = NULL;
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

int search(char c[64])
{
	int i=0,k=0,d=0;
	char curr[64]={0};
	char delim[] = "/";
	char *result = NULL;
	printf("Starting Search.\nInput is: %s\n", c);
	if( c[0]=='\0' )
	{
		printf("Input:%s is bad", c);
		return -1;
	} // c is bad mojo

	if ( c[0] == '/' )
	{
		printf("New directory is in root\n");
		wd = root;
		if(c[1] != '\0'){
			i=1;
		}else{return 1;}
	}else{
		printf("New Directory will be in current directory");
		wd = cwd;
	}
	result = strtok( c, delim );
	while( result != NULL ) // Loop the input
	{
		k = 0;
		/*while( c[i] != '/' && c[i] != '\0' ) 
		{
			curr[k] = c[i];	//first directory name
		}*/

		printf("Looking for %s in %s" , curr, wd->mName);
	
		if(wd->mType == 'D' && wd->mName == result)
		{//HIT
			return 1; 
		}else if(wd->s == '\0')
		{//Didn't match and no siblings, path doesn't exist
			return -1;
		}
		printf("Didn't match child");
		while(wd->s != '\0')
		{printf("Check sibling %s", wd->mName);
			wd = wd->s;
			if(wd->mType == 'D' && wd->mName == result)
			{
				return 1; 
			}else if(wd->s == '\0')
			{
				return -1;
			}
		}
	}
}

void mkdir()
{
	printf("**** mkdir ****\n");
	int d=0;
	int c=0, i=0;
	char curr[64] = {0};
	//dirname = {0};
	memset(dirname, 0, sizeof dirname);
	//basename = {0};
	memset(basename, 0, sizeof basename);
	while(1)
	{
		if ( pathname[i] == '\0' )
		{
			int k=0;
			for (k = 0; k < c; ++k)
			{ basename[k] = curr[k+1];} 
			dirname[i-c+1] = '\0'; 
			basename[k+1] = '\0';
			break;
		}
		if ( pathname[i] == '/')
		{ 
			c = 0;
		}
		dirname[i] = pathname[i];
		curr[c] = pathname[i];
		
		++c;
		++i;
	}
	i=0;
	printf("Dir= ");
	while(dirname[i] != '\0')
	{ printf("%c",dirname[i]); ++i;}
	i=0;
	printf("\tBasename= ");
	while(basename[i] != '\0')
	{ printf("%c",basename[i]); ++i;}
	printf("\n");	

	// go down dir till we reach the end
	// if all of it exists grab the last node as parent
	printf("Making tmp pointer");	
	struct node* tmp = malloc(sizeof *tmp);
	int exists = 1;
	exists = search(dirname);
	if ( exists == 1 ) // exists
	{
		printf("making new node\n");
		// Node work
		strcpy(tmp->mName, basename);
		printf("tmp name is: %s\n", tmp->mName);

		tmp->mType = 'D';
		tmp->s = NULL;
		
		if( wd->c )	// if parent->child is != null grab child
		{
			printf("Going to child of %s", wd->mName);
			wd = (wd->c)->s;
			while (wd->s != '\0'){wd=wd->s;}	// child -> sibling is this
			wd->s = tmp;
		}else
		{wd->c = tmp;}
	}else
	{ // No directory, print error, break
		printf("No Directory Exists\n");
	}
	
}

void rmdir()
{
	struct node* d = malloc ( sizeof *d);
	if ( d->c != NULL)
	{ rmdir(d->c);}
	(d->p)->c = NULL;	// parent no longer points here
	
}

void cd()
{
	
}

void ls()
{
	struct node *tmp = cwd;
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
}

void split()
{
	int i=0,pa=0;
	
	for(i = 0; i<128; ++i)
	{
		if (line[i] == '\0')
		{	
			command[i] = '\0';
			break;
		}
		if (line[i] == '/')
		{
			int pa=0;
			while (line[i] != '\0')
			{
				pathname[pa] = line[i];
				++i;
				++pa;	
			}
			pathname[pa] = '\0';
			break;			
		}
		else
		{
			command[i] = line[i];
		}
	}	
}

main()
{
	initialize();      /* initialize the / DIR of the tree */
	while(1){
	printf("%s: ", cwd->mName);
	gets(line);
	sSplit();
	//read a line containting  command [pathname]; // [ ] means optional
	//int ID = line[0]- '0';
	//int ID = findCommand(command);
	switch(command[0]){ 
		case 'm' :
		case 'h' :
			if(command[1] == 'e'){ menu(); }
			else{mkdir(); }   
			break;
		case 'r' : 
			rmdir();    
			break;
		case 'c' : cd();       break;
		case 'l': ls();       break;
		case 'e' : exit(EXIT_SUCCESS); break; 
		default:
			printf("Invalid"); break;
		}
	}
}
