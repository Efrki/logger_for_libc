#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "common.h"

static SharedLogBuffer *buf;
static int semid;
static int want_io, want_mem, interval=200;
static char of_io[256], of_mem[256];

int is_io(LogType t){
    return t<=LOG_TYPE_WRITE;
}
int is_mem(LogType t){
    return t>=LOG_TYPE_MALLOC;
}

void print(const LogEntry *e, FILE *o){
    long s=e->timestamp.tv_sec, ms=e->timestamp.tv_nsec/1000000;
    switch(e->type){
    case LOG_TYPE_OPEN:
        fprintf(o,"[%ld.%03ld] PID:%d %s open(\"%s\",%d)=%d\n",
            s,ms,e->pid,e->comm,
            e->data.open_log.filename,
            e->data.open_log.flags,
            e->data.open_log.fd);
        break;
    case LOG_TYPE_CLOSE:
        fprintf(o,"[%ld.%03ld] PID:%d %s close(%d)=%d\n",
            s,ms,e->pid,e->comm,
            e->data.close_log.fd,
            e->data.close_log.return_code);
        break;
    case LOG_TYPE_LSEEK:
        fprintf(o,"[%ld.%03ld] PID:%d %s lseek(%d,%ld,%d)=%ld\n",
            s,ms,e->pid,e->comm,
            e->data.lseek_log.fd,
            e->data.lseek_log.offset,
            e->data.lseek_log.whence,
            e->data.lseek_log.result);
        break;
    case LOG_TYPE_READ:
        fprintf(o,"[%ld.%03ld] PID:%d %s read(%d,%p,%zu)=%zd\n",
            s,ms,e->pid,e->comm,
            e->data.read_log.fd,
            e->data.read_log.buf_ptr,
            e->data.read_log.count,
            e->data.read_log.bytes_read);
        break;
    case LOG_TYPE_WRITE:
        fprintf(o,"[%ld.%03ld] PID:%d %s write(%d,%p,%zu)=%zd\n",
            s,ms,e->pid,e->comm,
            e->data.write_log.fd,
            e->data.write_log.buf_ptr,
            e->data.write_log.count,
            e->data.write_log.bytes_written);
        break;
    case LOG_TYPE_MALLOC:
        fprintf(o,"[%ld.%03ld] PID:%d %s malloc(%zu)=%p\n",
            s,ms,e->pid,e->comm,
            e->data.malloc_log.size,
            e->data.malloc_log.ptr);
        break;
    case LOG_TYPE_REALLOC:
        fprintf(o,"[%ld.%03ld] PID:%d %s realloc(%p,%zu)=%p\n",
            s,ms,e->pid,e->comm,
            e->data.realloc_log.old_ptr,
            e->data.realloc_log.size,
            e->data.realloc_log.new_ptr);
        break;
    case LOG_TYPE_FREE:
        fprintf(o,"[%ld.%03ld] PID:%d %s free(%p)\n",
            s,ms,e->pid,e->comm,
            e->data.free_log.ptr);
        break;
    }
    fflush(o);
}

int main(int c,char **v){
    static struct option o[] = {
        {"type",required_argument,0,'t'},
        {"interval",required_argument,0,'i'},
        {"io-log",required_argument,0,'o'},
        {"mem-log",required_argument,0,'m'},
        {0,0,0,0}
    };
    int ch;
    while((ch=getopt_long(c,v,"t:i:o:m:",o,0))!=-1){
        if(ch=='t'){
            if(strchr(optarg,'i')) want_io=1;
            if(strchr(optarg,'m')) want_mem=1;
        } else if(ch=='i') interval=atoi(optarg);
        else if(ch=='o') strncpy(of_io,optarg,255);
        else if(ch=='m') strncpy(of_mem,optarg,255);
    }
    if(!want_io&&!want_mem) return 1;
    key_t key=ftok(IPC_KEY_PATH,IPC_PROJ_ID);
    semid=semget(key,1,0666);
    int shmid=shmget(key,sizeof(*buf),0666);
    buf=shmat(shmid,0,0);
    FILE *fio=want_io?fopen(of_io,"a"):0;
    FILE *fmem=want_mem?fopen(of_mem,"a"):0;
    size_t tail=buf->tail;
    struct sembuf sb={0,-1,0}, sb2={0,1,0};
    for(;;){
        semop(semid,&sb,1);
        while(tail!=buf->head){
            const LogEntry *e=&buf->entries[tail];
            if(want_io&&is_io(e->type)) print(e,fio);
            if(want_mem&&is_mem(e->type)) print(e,fmem);
            tail=(tail+1)%LOG_BUFFER_CAPACITY;
        }
        buf->tail=tail;
        semop(semid,&sb2,1);
        usleep(interval*1000);
    }
    return 0;
}
