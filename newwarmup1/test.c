#include <stdio.h>
#include <math.h>

int main()
{
   char str[80];

   sprintf(str, "str = %.2f", 1222.456);
   strcpy(str, "What to do when you dont have \n");

   puts(str);
	printf("%-24.24s",str);
	printf("EOF\n");
   
   return(0);
}
