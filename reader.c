#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

// Helper function to perform semaphore operations. Exits on failure.
void semaphore_op(int sem_id, int op) {
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = op;
    sops.sem_flg = 0;
    if (semop(sem_id, &sops, 1) == -1) {
        perror("semop");
        exit(SEMOP_ERROR);
    }
}

void lock_semaphore(int sem_id){
    semaphore_op(sem_id, -1);
}

void unlock_semaphore(int sem_id){
    semaphore_op(sem_id, 1); 
}

void process_log_entry(const LogEntry* entry){
    printf("Reader: Got log entry from pid %d, type %d at time %ld\n",entry->pid, entry->type, entry->timestamp.tv_sec);
}

int main(int argc, char* argv[]){
    (void)argc;
    (void)argv;

    key_t key = ftok(IPC_KEY_PATH, IPC_PROJ_ID);
    if (key == -1) {
        perror("ftok");
        return FTOK_ERROR;
    }

    int sem_id = semget(key, 1, 0666);
    if (sem_id == -1) {
        perror("semget: Is the logger library running?");
        return SEMGET_ERROR;
    }

    int shm_id = shmget(key, sizeof(SharedLogBuffer), 0666);
    if (shm_id == -1) {
        perror("shmget: Is the logger library running?");
        return SHMGET_ERROR;
    }

    SharedLogBuffer* buffer = (SharedLogBuffer*)shmat(shm_id, NULL, 0); 
    if (buffer == (void*)-1) {
        perror("shmat");
        return SHMAT_ERROR;
    }
    
    printf("Reader daemon started. Waiting for logs...\n");

    size_t current_tail = buffer->tail;

    while (1) {
        lock_semaphore(sem_id);

        if (current_tail != buffer->head) {

            while (current_tail != buffer->head) {
                process_log_entry(&buffer->entries[current_tail]);
                current_tail = (current_tail + 1) % LOG_BUFFER_CAPACITY;
            }
            buffer->tail = current_tail;
        }
        unlock_semaphore(sem_id);

        // TODO: Make this sleep interval configurable via command-line arguments
        usleep(500000);
    }

    shmdt(buffer);

    return 0;
}