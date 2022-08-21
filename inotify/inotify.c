#include <sys/inotify.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h> 

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

time_t mtime, last_mtime;


void printInotifyEvent(struct inotify_event* event, char* filename){
    struct stat stat_buf;
    assert(stat(filename,&stat_buf)==0);

    if (event->mask & IN_MODIFY){
        last_mtime = mtime;
        mtime = stat_buf.st_mtime;
        printf("new mtime : %s",asctime(gmtime(&mtime))); 
        printf("last mtime : %s",asctime(gmtime(&last_mtime)));
        // char command[1024] = "cat ";
        // strcat(command,event->name); 
        // system(command);
        printf("Modify file %s\n", event->name );
        printf("\n\n");
    }
    else if(event->mask & IN_CREATE ){
        last_mtime = mtime;
        mtime = stat_buf.st_mtime;
        printf("new mtime : %s",asctime(gmtime(&mtime))); 
        printf("last mtime : %s",asctime(gmtime(&last_mtime)));
        if(event->mask & IN_ISDIR){
            printf( "New directory %s created.\n", event->name );
        }
        else{
            printf( "New file %s created.\n", event->name );
        }
        printf("\n\n");
    }
    else if(event->mask & IN_DELETE){
        last_mtime = mtime;
        mtime = stat_buf.st_mtime;
        printf("new mtime : %s",asctime(gmtime(&mtime))); 
        printf("last mtime : %s",asctime(gmtime(&last_mtime)));
        if ( event->mask & IN_ISDIR ) {
            printf( "Directory %s deleted.\n", event->name );
        }
        else {
            printf( "File %s deleted.\n", event->name );
        }
        printf("\n\n");
    }
}

int main(int argc, char** argv){
    int fd, num, i, wd;
    char* p;
    char inotify_entity[BUF_LEN];
    struct stat stat_buf;

    fd = inotify_init();
    wd = inotify_add_watch(fd, argv[1] ,IN_ALL_EVENTS);

    // print mtime & content while exe
    assert(stat(argv[1],&stat_buf)==0);
    mtime = stat_buf.st_mtime;
    printf("%s\n",asctime(gmtime(&mtime))); 

    while(1){
        num = read(fd, inotify_entity, BUF_LEN);
        for (p = inotify_entity; p < inotify_entity + num; ) {
			printInotifyEvent((struct inotify_event *) p, argv[1]);
			p+=sizeof(struct inotify_event) + ((struct inotify_event *)p)->len;
		}
    }
    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}