#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdarg.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include "common.h"
#include <stdio.h>

static SharedLogBuffer *buf;
static int semid;
static int init_done;
static char proc_name[MAX_COMM_LEN];

static void sop(int op){
    struct sembuf sb = {0, op, SEM_UNDO};
    semop(semid, &sb, 1);
}

static void lock(){ sop(-1); }
static void unlock(){ sop(1); }

__attribute__((constructor))
static void initlib(){
    proc_name[0]=0;
    FILE *f = fopen("/proc/self/comm","r");
    if(f){
        fgets(proc_name, MAX_COMM_LEN, f);
        proc_name[strcspn(proc_name,"\n")]=0;
        fclose(f);
    }
    init_done = 0;
    key_t key = ftok(IPC_KEY_PATH, IPC_PROJ_ID);
    int shmid = shmget(key, sizeof(*buf), IPC_CREAT|0666);
    buf = shmat(shmid, NULL, 0);
    semid = semget(key,1,IPC_CREAT|0666);
    semctl(semid,0,SETVAL,1);
    buf->head=buf->tail=0;
    init_done = 1;
}

static LogEntry *new_ent(){
    if(!init_done||!buf) return 0;
    lock();
    size_t nh=(buf->head+1)%LOG_BUFFER_CAPACITY;
    if(nh==buf->tail){ unlock(); return 0; }
    LogEntry *e=&buf->entries[buf->head];
    e->pid=getpid();
    memcpy(e->comm,proc_name,MAX_COMM_LEN);
    clock_gettime(CLOCK_REALTIME,&e->timestamp);
    return e;
}

static void push(){ buf->head=(buf->head+1)%LOG_BUFFER_CAPACITY; unlock(); }

int open(const char *p, int f, ...){
    mode_t m=0;
    if(f&O_CREAT){ va_list a; va_start(a,f); m=va_arg(a,mode_t); va_end(a); }
    static int (*ro)(const char*,int,mode_t);
    if(!ro) ro=dlsym(RTLD_NEXT,"open");
    int fd=ro(p,f,m);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_OPEN;
        strncpy(e->data.open_log.filename,p,MAX_PATH_LEN-1);
        e->data.open_log.flags=f;
        e->data.open_log.fd=fd;
        push();
    }
    return fd;
}

int close(int fd){
    static int (*rc)(int);
    if(!rc) rc=dlsym(RTLD_NEXT,"close");
    int r=rc(fd);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_CLOSE;
        e->data.close_log.fd=fd;
        e->data.close_log.return_code=r;
        push();
    }
    return r;
}

off_t lseek(int fd, off_t o, int w){
    static off_t (*rl)(int,off_t,int);
    if(!rl) rl=dlsym(RTLD_NEXT,"lseek");
    off_t r=rl(fd,o,w);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_LSEEK;
        e->data.lseek_log.fd=fd;
        e->data.lseek_log.offset=o;
        e->data.lseek_log.whence=w;
        e->data.lseek_log.result=r;
        push();
    }
    return r;
}

ssize_t read(int fd, void *b, size_t c){
    static ssize_t (*rr)(int,void*,size_t);
    if(!rr) rr=dlsym(RTLD_NEXT,"read");
    ssize_t r=rr(fd,b,c);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_READ;
        e->data.read_log.fd=fd;
        e->data.read_log.buf_ptr=b;
        e->data.read_log.count=c;
        e->data.read_log.bytes_read=r;
        push();
    }
    return r;
}

ssize_t write(int fd, const void *b, size_t c){
    static ssize_t (*rw)(int,const void*,size_t);
    if(!rw) rw=dlsym(RTLD_NEXT,"write");
    ssize_t r=rw(fd,b,c);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_WRITE;
        e->data.write_log.fd=fd;
        e->data.write_log.buf_ptr=b;
        e->data.write_log.count=c;
        e->data.write_log.bytes_written=r;
        push();
    }
    return r;
}

void *malloc(size_t s){
    static void *(*rm)(size_t);
    if(!rm) rm=dlsym(RTLD_NEXT,"malloc");
    void *p=rm(s);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_MALLOC;
        e->data.malloc_log.size=s;
        e->data.malloc_log.ptr=p;
        push();
    }
    return p;
}

void *realloc(void *o, size_t s){
    static void *(*rrt)(void*,size_t);
    if(!rrt) rrt=dlsym(RTLD_NEXT,"realloc");
    void *p=rrt(o,s);
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_REALLOC;
        e->data.realloc_log.old_ptr=o;
        e->data.realloc_log.size=s;
        e->data.realloc_log.new_ptr=p;
        push();
    }
    return p;
}

void free(void *p){
    static void (*rf)(void*);
    if(!rf) rf=dlsym(RTLD_NEXT,"free");
    LogEntry *e=new_ent();
    if(e){
        e->type=LOG_TYPE_FREE;
        e->data.free_log.ptr=p;
        push();
    }
    rf(p);
}
