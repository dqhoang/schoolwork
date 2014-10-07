#include <stdio.h>
/**************** t.c file ************************/
main(int argc, char *argv[], char *env[])
{
  int a,b,c;
  printf("enter main\n");
  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}

int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  d=3; e=4; f=5;
  B(d,e);
  printf("exit A\n");
}

int B(int x, int y)
{
  int g,h,i;
  printf("enter B\n");
  g=6; h=7; i=8;
  C(g,h);
  printf("exit B\n");
}

int C(int x, int y)
{
  int u,v,w, *fpC, *fp0; 
  printf("enter C\n");
  u=9; v=10; w=11;

    /* Write C and assembly code to 
    ********* DO (1)-(4) AS SPECIFIED BELOW *************/

  fp0 = get_esp();
  fpC = get_ebp();
  int i;
printf("\nHex Address\tContents\n");
printf("=============================\n");
while(fpC != 0){
  	printf("fpC=%8x\t%8u\n", fp0, *fp0 );
	++fp0;
	if ( fp0 == fpC )
	{
		printf("Moving to next frame stack\n");
		printf("---------------------------\n");
		fpC= (*fpC);
	}
}
  printf("exit C\n");
}
