#include "stdio.h"
#include "string.h"

int equal(char* a, char* b)
{
    int i = 0;
    int is_equal = 1;
    if (strlen(a) != strlen(b)) {
        return 0;
    }
    for (i = 0; i < strlen(a); i++) {
        if (a[i] != b[i]) {
            is_equal = 0;
            break;
        }
    }
    return is_equal;
}

int main(int args, char* argv[]) {
    if (equal(argv[1], "TTY") || equal(argv[1], "SYS") || equal(argv[1], "HD") || equal(argv[1], "FS") || equal(argv[1], "MM") || equal(argv[1], "INIT") || equal(argv[1], "TestB") || equal(argv[1], "TestC")) {
        printf("%s can't be killed.\n", argv[1]);
    } else {
        kill(argv[1]);
    }
    return 0;
}
