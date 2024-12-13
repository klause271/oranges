#include<stdio.h>
#include<string.h>

int main(int argc, char *argv[]){
    char num[4][32] = {"2021302181189","2021302181075","2021302191707","2021302191289"};
    char name[4][32] = {"zhanxuan","Liuweifeng","Duankaiwen","Fuyankai"};

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
