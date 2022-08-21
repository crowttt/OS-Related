#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#define BUF_SIZE 4096

 
int main(int argc, char* argv[]) {
    //從inputFd將檔案寫到outputFd
    int inputFd, outputFd;
    //讀進多少，寫出多少
    ssize_t numIn, numOut;
    //把檔案內容讀到buffer，再寫出去
    char buffer[BUF_SIZE];

    //確定使用者輸入二個參數
    if (argc != 3) {
        char* filename=basename(argv[0]);
        printf("『%s』的功能是檔案複製，要有二個參數，來源檔案和目標檔案\n", filename);
        exit(0);
    }
 
    //打開來源檔案
    inputFd = open (argv [1], O_RDONLY);
    if (inputFd == -1) {
        char* filename=basename(argv[1]);
        char errmsg[1024];
        sprintf(errmsg, "無法開啟來源檔案 (%s)", filename);
        perror (errmsg); 
        exit(1); 
    }
 
    //打開目的檔案
    //注意 open 的參數，可讀、創造、歸零（O_WRONLY | O_CREAT | O_TRUNC）
    //比較常忘記的是歸零，是否歸零是情況而定
    //如果沒有歸零就會有新舊混淆的問題
    //資料庫系統（DBMS）打開打檔案通常不會歸零，因為他們會在既有的上面做更新
    //word打開檔案通常要歸零，因為使用者的新文章可能更短，這會造成新舊混淆問題
    outputFd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR| S_IWUSR);
    if(outputFd == -1){
        char* filename=basename(argv[3]);
        char errmsg[1024];
        sprintf(errmsg, "無法打開目的檔案 (%s)", filename);
        perror (errmsg); 
        exit(1); 
    }

    off_t data_off=0, hole_off=0, cur_off=0;
    long long fileSize, blockSize, pos=0;
    fileSize = lseek(inputFd, 0, SEEK_END);
    lseek(inputFd, 0, SEEK_SET);

    struct timespec timer1, timer2;
    clock_gettime(CLOCK_MONOTONIC,&timer1);

	while (1) {
		cur_off = lseek(inputFd, cur_off, SEEK_DATA);
        data_off = cur_off;
		cur_off = lseek(inputFd, cur_off, SEEK_HOLE);
        hole_off = cur_off;
        //第一種情況，資料在前面，洞在後面，不用特別處理
        //第二種情況，洞在前面，資料在後面，處理一下
        if (data_off > hole_off) {
            //現在是這樣：
            //  ...............data data data data data....
            //  ^hole_off      ^data_off=cur_off
            //因為cur_off已經移到後面了，所以下一輪執行會變成
            //  ...............data data data data data....
            //                 ^data_off               ^hole_off=curoff
            continue;
        }

		blockSize=hole_off-data_off;
		lseek(inputFd, data_off, SEEK_SET);
		lseek(outputFd, data_off, SEEK_SET);
        //這個while loop與 mycp 相同
		while((numIn = read (inputFd, buffer, BUF_SIZE)) > 0) {
			numOut = write (outputFd, buffer, (ssize_t) numIn);
			if (numIn != numOut) perror("numIn != numOut");
			blockSize-=numIn;
			if (blockSize == 0) break;
		}
        //檢查一下是否已經到最後了
		if (lseek(outputFd, 0, SEEK_CUR) == fileSize) break;
    }

    clock_gettime(CLOCK_MONOTONIC,&timer2);
    printf("time_consume :%ld ns\n",(timer2.tv_sec*1000000000 + timer2.tv_nsec) - (timer1.tv_sec*1000000000 + timer1.tv_nsec));


    close (inputFd);
    close (outputFd);
 
    return (EXIT_SUCCESS);
}

