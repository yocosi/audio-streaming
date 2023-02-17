#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "audio.h"

int main(int argc, char *argv[]){
    int a, b, c;

    int *ptrA = &a;
    int *ptrB = &b;
    int *ptrC = &c; 
    char buffer[1024];

    int fd = aud_readinit (argv[1], ptrA, ptrB, ptrC); 

    if(fd == -1){
        perror("Error: readinit.");
        exit(1);
    }

    a = 44100;
    b = 16;
    c = 2;

    int wtInit = aud_writeinit (a, b, c);
    if(wtInit == -1){
        perror("Error: writeinit.");
        exit(1);
    }

    while(read(fd, &buffer, sizeof(buffer))>0){
        write(wtInit, &buffer, sizeof(buffer));
    }
    close(fd);
    close(wtInit);
    return 0;
}