#include<stdio.h>
#include<string.h>

int main(int argc, char *argv[]){
    char num[2][32] = {"2022302181189","2022302181191"};
    char name[2][32] = {"zhanxuan","wanliwei"};

int i;
    for (i = 0; i < 4 ; i++) {

        if (strcmp(argv[1],num[i]) == 0) {
            printf("Yes! %s %s is one of our members!\n",num[i],name[i]);
	    return 0;
        }
        else if(i==3){
            printf("No! This student is not in our members!\n");
            return 0;
        }
    }
}
