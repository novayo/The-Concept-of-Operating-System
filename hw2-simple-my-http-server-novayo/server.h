#ifndef SERVER_H
#define SERVER_H
#include "status.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#define MAX_QUEUE 1000
#define BUFFER 9999
#define TMPBUFFER 200

typedef struct tpool_work {
    void (*routine)();
    void *arg;
    struct tpool_work *next;
} tpool_work_t;

typedef struct tpool {
    int nthreads;
    int max_queue;
    int queue_size;

    pthread_t *threads;
    tpool_work_t *queue_head;
    tpool_work_t *queue_tail;
    pthread_mutex_t queue_lock;
    pthread_cond_t  queue_not_empty;
    pthread_cond_t  queue_not_full;
    pthread_cond_t  queue_empty;
} *tpool_t;

void tpool_init(tpool_t *tpoolp, int nthreads, int max_queue);
int tpool_add_work(tpool_t tpool, void *routine, void *arg);
void *tpool_thread(void *arg);

void tpool_init(tpool_t *tpoolp, int nthreads, int max_queue)
{
    tpool_t tpool;

    // Initial thread pool
    tpool = (tpool_t )malloc(sizeof(struct tpool));
    tpool->nthreads = nthreads;
    tpool->max_queue = max_queue;
    tpool->threads = (pthread_t *)malloc(sizeof(pthread_t)*nthreads);
    tpool->queue_size = 0;
    tpool->queue_head = NULL;
    tpool->queue_tail = NULL;
    pthread_mutex_init(&(tpool->queue_lock), NULL);
    pthread_cond_init(&(tpool->queue_not_empty), NULL);
    pthread_cond_init(&(tpool->queue_not_full), NULL);
    pthread_cond_init(&(tpool->queue_empty), NULL);

    // Thread create
    int i=0;
    for (i=0; i != nthreads; i++) {
        pthread_create(&(tpool->threads[i]), NULL, tpool_thread, (void *)tpool);
    }
    *tpoolp = tpool;
}

void *tpool_thread(void *arg)
{
    tpool_t tpool = (tpool_t) arg;
    tpool_work_t *my_workp;

    // Stuck thread
    while(1) {
        pthread_mutex_lock(&(tpool->queue_lock));
        while (tpool->queue_size == 0) {
            pthread_cond_wait(&(tpool->queue_not_empty), &(tpool->queue_lock));
        }

        // grab task
        my_workp = tpool->queue_head;
        tpool->queue_size--;
        if (tpool->queue_size == 0)	tpool->queue_head = tpool->queue_tail = NULL;
        else tpool->queue_head = my_workp->next;

        if (tpool->queue_size == (tpool->max_queue - 1))
            pthread_cond_broadcast(&(tpool->queue_not_full));

        if (tpool->queue_size == 0)
            pthread_cond_signal(&(tpool->queue_empty));

        pthread_mutex_unlock(&(tpool->queue_lock));
        (*(my_workp->routine))(my_workp->arg);
        free(my_workp);
    }
}

int tpool_add_work(tpool_t tpool, void *routine, void *arg)
{
    pthread_mutex_lock(&tpool->queue_lock);
    tpool_work_t *workp;

    if (tpool->queue_size == tpool->max_queue) {
        pthread_mutex_unlock(&tpool->queue_lock);
        return -1;
    }
    while (tpool->queue_size == tpool->max_queue) {
        pthread_cond_wait(&tpool->queue_not_full, &tpool->queue_lock);
    }

    workp = (tpool_work_t *)malloc(sizeof(tpool_work_t));
    workp->routine = routine;
    workp->arg = arg;
    workp->next = NULL;
    if (tpool->queue_size == 0) {
        tpool->queue_tail = tpool->queue_head = workp;
        pthread_cond_broadcast(&tpool->queue_not_empty);
    } else {
        (tpool->queue_tail)->next = workp;
        tpool->queue_tail = workp;
    }
    tpool->queue_size++;
    pthread_mutex_unlock(&tpool->queue_lock);
    return 1;
}

char *get_SubDir(char *target, int do_not_find_file)
{
    char *s = malloc(BUFFER);
    memset(s, '\0', BUFFER);
    // for find dir
    if (do_not_find_file) {
        struct dirent *ptr_Dir = NULL;
        DIR *Dir = opendir(target);
        if (Dir == NULL) {
            char status[TMPBUFFER] = {'\0'};
            sprintf(status, "%d", status_code[NOT_FOUND]);
            strcat(s, status);
            strcat(s, " ");
            strcat(s, "NOT_FOUND|");
        } else {
            while((ptr_Dir = readdir(Dir)) != NULL) {
                if (!strcmp(ptr_Dir->d_name, ".") || !strcmp(ptr_Dir->d_name, "..")) continue;
                strcat(s, ptr_Dir->d_name);
                strcat(s, " ");
            }
        }
        free(ptr_Dir);
        closedir(Dir);
    }
    // for cat file
    else {
        char ch;
        FILE *fp;
        fp = fopen(target, "r");
        if (fp) {
            while((ch = fgetc(fp)) != EOF) {
                int len = strlen(s);
                s[len + 1] = s[len];
                s[len] = ch;
            }
            fclose(fp);
        }
        // if can't find file
        else {
            char status[TMPBUFFER] = {'\0'};
            sprintf(status, "%d", status_code[NOT_FOUND]);
            strcat(s, status);
            strcat(s, " ");
            strcat(s, "NOT_FOUND|");
        }
    }
    return s;
}
#endif
