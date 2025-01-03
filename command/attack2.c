#include "stdio.h"

int main(int argc, char * argv[])
{
    int i = 1;
    int a[12] = {2,0,2,2,3,0,2,1,8,1,1,8,9};
    for(i; i < argc; i++)
    {
	    printf(argv[i]);
        printf(" ");
    }
	return 0;
}
