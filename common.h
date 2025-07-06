#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>

#define IPC_KEY_PATH "/tmp"
#define IPC_PROJ_ID 'L'

#define LOG_BUFFER_CAPACITY 256
#define MAX_PATH_LEN 256
#define MAX_COMM_LEN 16

typedef enum {
    LOG_TYPE_OPEN,
    LOG_TYPE_CLOSE,
    LOG_TYPE_LSEEK,
    LOG_TYPE_READ,
    LOG_TYPE_WRITE,
    LOG_TYPE_MALLOC,
    LOG_TYPE_REALLOC,
    LOG_TYPE_FREE,

} LogType;

typedef enum {
    SEMOP_ERROR = 1,
    FTOK_ERROR,
    SEMGET_ERROR,
    SHMGET_ERROR,
    SHMAT_ERROR
} Errors;

typedef struct {
    char filename[MAX_PATH_LEN];
    int flags;
    int fd;
} OpenLog;

typedef struct {
    int fd;
    int return_code;
} CloseLog;

typedef struct {
    int fd;
    off_t offset;
    int whence;
    off_t result;
} LseekLog;

typedef struct {
    int fd;
    void* buf_ptr;
    size_t count;
    ssize_t bytes_read;
} ReadLog;

typedef struct {
    int fd;
    const void* buf_ptr;
    size_t count;
    ssize_t bytes_written;
} WriteLog;

typedef struct {
    size_t size;
    void* ptr;
} MallocLog;

typedef struct {
    void* old_ptr;
    size_t size;
    void* new_ptr;
} ReallocLog;

typedef struct {
    void* ptr;
} FreeLog;

typedef struct {
    pid_t pid;
    char comm[MAX_COMM_LEN];
    struct timespec timestamp;
    LogType type;

    union{
        OpenLog open_log;
        CloseLog close_log;
        LseekLog lseek_log;
        ReadLog read_log;
        WriteLog write_log;
        MallocLog malloc_log;
        ReallocLog realloc_log;
        FreeLog free_log;
    } data;
} LogEntry;

typedef struct {
    volatile size_t head;
    volatile size_t tail;

    LogEntry entries[LOG_BUFFER_CAPACITY];
} SharedLogBuffer;

#endif // COMMON_H
