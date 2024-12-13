#include "stdio.h"
#include "type.h"

int main(int argc, char *argv[]){

    char *result;
    result = slist("/");
    printf("%s",result);
    printf("\n");

    return 0;
}
