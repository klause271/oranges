#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        printf("Usage: cat filename\n");
        return 0;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        printf("File not found: %s\n", argv[1]);
        return 0;
    }

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char buffer[1024] = { 0 };
    while (size > 0)
    {
        int read_size = read(fd, buffer, 1024);
        size -= read_size;
        write(1, buffer, read_size);
    }

   
    close(fd);

    return 0;
}
