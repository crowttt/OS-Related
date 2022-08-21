#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 4096

int main(int argc, char** argv){
    int lockfDbFd;
    int numIn;
    //char buffer[BUF_SIZE] = {0};
    char* buffer = calloc(BUF_SIZE,sizeof(char));
    int off;

    //lockfDbFd = open("./lockf.db",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    lockfDbFd = open("./lockf.db",O_WRONLY);

    for(int i=0;i<1000;i++){
        lseek(lockfDbFd,0, SEEK_SET); // 移到檔案起始位置
        lockf(lockfDbFd, F_LOCK, 0);
        lseek(lockfDbFd,0,SEEK_END);
        lseek(lockfDbFd,-4,SEEK_END);

        read(lockfDbFd,buffer,4);

        sscanf(buffer,"%d",&off);
        lseek(lockfDbFd,0,SEEK_END);
        lseek(lockfDbFd,off,SEEK_CUR);
        
        sprintf(buffer,"%d",off+1);

        write(lockfDbFd,buffer,strlen(buffer));
        lockf(lockfDbFd, F_UNLCK, 0);
        usleep(100000);
    
    }
    close(lockfDbFd);
}