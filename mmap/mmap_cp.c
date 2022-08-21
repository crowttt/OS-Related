#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>

 
int main(int argc, char* argv[]) {
 
    int inputFd, outputFd;
    char *inputPtr, *outputPtr;
    ssize_t numIn, numOut;
    ssize_t fileSize=0;
    off_t cur_off=0, hole_off=0, data_off=0;
    long long blockSize;
    //char buffer[BUF_SIZE];
 
    //只可讀取模式打開
    inputFd = open (argv [1], O_RDONLY);
    if (inputFd == -1) {
        perror ("cannot open the file for read"); exit(1); }
 
    //open後可對該檔案＊＊『可讀可寫』＊＊（因為mmap的需求），如果沒有該檔案，就建立該檔案。如果要建立，設定該檔案的屬性為owner可讀可寫
    outputFd = open(argv[2], O_RDWR | O_CREAT, S_IRUSR| S_IWUSR);
    if(outputFd == -1){
        perror("canot open the file for write"); exit(1); }

    //lseek的回傳是該檔案的絕對位址，因此lseek(0, seek_end)相當於檔案大小
    //linux有專門讀取檔案大小的函數，但我習慣用這一個
    fileSize = lseek(inputFd, 0, SEEK_END);
    lseek(inputFd, 0, SEEK_SET);
    printf("file size = %ld\n", fileSize);

    //NULL，不指定映射到記憶體的哪個位置。通常不指定
    //filesize，將檔案中多少內容映射到記憶體
    //prot_read，只會對該段記憶體做讀取
    //MAP_SHARED，對mmap出的記憶體的所有修改讓整個系統裡的人都看到。因此底藏的檔案會跟著變更
    //inputFd從哪個檔案映射進來
    //0, 映射的起點為 0
    inputPtr = mmap(NULL, fileSize, PROT_READ, MAP_SHARED , inputFd , 0);//🐶 🐱 🐭 🐹 🐰 🦊
    perror("mmap");
    printf("inputPtr = %p\n", inputPtr);
    //assert(madvise(inputPtr, fileSize, MADV_SEQUENTIAL|MADV_WILLNEED|MADV_HUGEPAGE)==0);

    //ftruncate的名字是：縮小
    //實際上是設定檔案大小
    ftruncate(outputFd, fileSize);  //🐶 🐱 🐭 🐹 🐰 🦊
    outputPtr = mmap(NULL, fileSize, PROT_WRITE, MAP_SHARED , outputFd , 0); //🐶 🐱 🐭 🐹 🐰 🦊
    perror("mmap, output");
    printf("outputPtr = %p\n", outputPtr);
    //madvise(inputPtr, fileSize, MADV_SEQUENTIAL|MADV_WILLNEED|MADV_HUGEPAGE);

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
        memcpy(outputPtr + data_off, inputPtr + data_off, blockSize);
		lseek(inputFd, data_off, SEEK_SET);
		lseek(outputFd, data_off, SEEK_SET);
        lseek(outputFd, cur_off, SEEK_SET);

        //檢查一下是否已經到最後了
		if (lseek(outputFd, 0, SEEK_CUR) == fileSize) break;
    }

    clock_gettime(CLOCK_MONOTONIC,&timer2);
    printf("time_consume :%ld ns\n",(timer2.tv_sec*1000000000 + timer2.tv_nsec) - (timer1.tv_sec*1000000000 + timer1.tv_nsec));

    assert(munmap(inputPtr, fileSize) == 0);
    assert(munmap(outputPtr, fileSize) == 0);
    
    assert(close (inputFd) == 0);
    assert(close (outputFd) == 0);

    return (EXIT_SUCCESS);
}