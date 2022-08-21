#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 4096

int main(int argc, char** argv){
    int flockDbFd;
    int numIn;
    char* buffer = calloc(BUF_SIZE,sizeof(char));
    //[BUF_SIZE] = {0};
    int off;

    //flockDbFd = open("./flock.db",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    flockDbFd = open("./flock.db",O_WRONLY);
    for(int i=0;i<1000;i++){
        //lseek(flockDbFd,0, SEEK_SET);
        flock(flockDbFd,LOCK_EX);
        lseek(flockDbFd,0,SEEK_END);
        lseek(flockDbFd,-4,SEEK_END);

        read(flockDbFd,buffer,4);

        sscanf(buffer,"%d",&off);
        lseek(flockDbFd,0,SEEK_END);
        lseek(flockDbFd,off,SEEK_CUR);
        
        sprintf(buffer,"%d",off+1);

        write(flockDbFd,buffer,strlen(buffer));
        flock(flockDbFd,LOCK_UN);
        usleep(100000);
    }
    close(flockDbFd);
}