#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(){
    int fd=open("out.txt",O_CREAT|O_WRONLY|O_TRUNC,0644);
    write(fd,"hello\n",6);
    lseek(fd,0,SEEK_SET);
    close(fd);

    char *p=malloc(32);
    strcpy(p,"memtest");
    p=realloc(p,64);
    free(p);

    printf("done\n");
    return 0;
}
