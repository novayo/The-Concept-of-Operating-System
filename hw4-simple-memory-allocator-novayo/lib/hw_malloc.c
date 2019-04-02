#include "hw_malloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <math.h>
#define BIN_SIZE 11

/* Global Variables */
void *start_brk = NULL;
Bool is_init = False;
bin_t s_bin[BIN_SIZE] = {};
bin_t *bin[BIN_SIZE];
int num_chunk = 0;

chunk_header_t *create_chunk(chunk_header_t *base, ll size);
chunk_header_t *split(chunk_header_t **ori, ll size);
chunk_header_t *merge(chunk_header_t *h);
ll best_fit_size(ll bytes);
void show_bin(int i);
void show_all_bin();
int search_debin(ll size);
int search_enbin(ll size);
void en_bin(int index, chunk_header_t *c_h);
chunk_header_t *de_bin(int index, ll size);
void rm_chunk_from_bin(chunk_header_t *c);
int check_valid_free(void *a_mem);

void *hw_malloc(size_t bytes)
{
    ll size = best_fit_size(bytes);
    if (is_init == False) {
        start_brk = sbrk(64 * 1024);
        int i=0;
        for (i = 0; i < BIN_SIZE; i++) {
            bin[i] = &s_bin[i];
            bin[i]->prev = bin[i];
            bin[i]->next = bin[i];
            bin[i]->size = 0;
        }
        chunk_header_t *s = create_chunk(get_start_sbrk(), 64 * 1024);
        chunk_header_t *c = split(&s, size);
        c = create_chunk(c, size);
        return (void *)((intptr_t)(void*)c + sizeof(chunk_header_t) - (intptr_t)(void*)get_start_sbrk());
    } else {
        chunk_header_t *r = NULL;
        //printf("size = %d\n", size);
        r = de_bin(search_debin(size), size);
        //printf("%ld - %ld = %ld\n", (intptr_t)(void*)r, (intptr_t)(void*)get_start_sbrk(), (intptr_t)(void*)r-(intptr_t)(void*)get_start_sbrk());
        return (void *)((intptr_t)(void*)r + sizeof(chunk_header_t) - (intptr_t)(void*)get_start_sbrk());
    }
    return NULL;
}

int hw_free(void *mem)
{
    void *a_mem = (void *)((intptr_t)(void*)mem + (intptr_t)(void*)get_start_sbrk());
    if (is_init == False || check_valid_free(a_mem) == False) {
        //printf("N|check_valid_free(a_mem) = %ld\n", (intptr_t)(a_mem));
        return False;
    } else {
        //printf("Y|check_valid_free(a_mem) = %d\n", check_valid_free(a_mem));
        chunk_header_t *h = (chunk_header_t *)((intptr_t)(void*)a_mem - (intptr_t)(void*)sizeof(chunk_header_t));
        chunk_header_t *nxt = (chunk_header_t *)((intptr_t)(void*)h + (intptr_t)((chunk_header_t *)h)->size_and_flag.current_chunk_size);
        nxt->size_and_flag.allocated_flag = True;
        chunk_header_t *m = merge(h);
        en_bin(search_enbin(m->size_and_flag.current_chunk_size), m);
        return True;
    }
}

void *get_start_sbrk(void)
{
    return (void *)start_brk;
}

chunk_header_t *create_chunk(chunk_header_t *base, ll size)
{
    if ((void *)base - get_start_sbrk() + size > 64 * 1024) {
        printf("heap not enough\n");
        return NULL;
    }
    chunk_header_t *ret = base;
    ret->size_and_flag.current_chunk_size = size;
    ret->size_and_flag.prev_chunk_size = size;
    ret->size_and_flag.allocated_flag = False;
    ret->size_and_flag.mmap_flag = False;
    ret->next = NULL;
    ret->prev = NULL;
    return ret;
}

chunk_header_t *split(chunk_header_t **ori, ll size)
{
    chunk_header_t *tmp_chunk;
    if (is_init == False) {
        is_init = True;
        tmp_chunk = (void *)(intptr_t)((void *)(*ori) + (int)pow(2.0, 15));
        tmp_chunk->size_and_flag.current_chunk_size = (int)pow(2.0, 15);
        tmp_chunk->size_and_flag.prev_chunk_size = (int)pow(2.0, 15);
        tmp_chunk->size_and_flag.allocated_flag = False;
        tmp_chunk->size_and_flag.mmap_flag = False;
        tmp_chunk->next = NULL;
        tmp_chunk->prev = NULL;
        en_bin(search_enbin(tmp_chunk->size_and_flag.current_chunk_size), tmp_chunk);
        num_chunk++;
    }
    int n = (*ori)->size_and_flag.current_chunk_size;
    if (n == (int)pow(2.0, 16)) n = (int)pow(2.0, 15);
    while(n > size) {
        n/=2;
        tmp_chunk = (void *)(intptr_t)((void *)(*ori) + n);
        tmp_chunk->size_and_flag.current_chunk_size = n;
        tmp_chunk->size_and_flag.prev_chunk_size = n;
        tmp_chunk->size_and_flag.allocated_flag = False;
        tmp_chunk->size_and_flag.mmap_flag = False;
        tmp_chunk->next = NULL;
        tmp_chunk->prev = NULL;
        en_bin(search_enbin(tmp_chunk->size_and_flag.current_chunk_size), tmp_chunk);
        num_chunk++;
    }
    //chunk_header_t *ret = create_chunk((*ori), size);
    return (*ori);
}

chunk_header_t *merge(chunk_header_t *h)
{
    chunk_header_t *nxt = (chunk_header_t *)((intptr_t)(void*)h + (intptr_t)((chunk_header_t *)h)->size_and_flag.current_chunk_size);
    chunk_header_t *nnxt = (chunk_header_t *)((intptr_t)(void*)nxt + (intptr_t)((chunk_header_t *)nxt)->size_and_flag.current_chunk_size);
    if (nnxt->size_and_flag.allocated_flag == True) {
        /*If next chunk is free, being able to merge*/
        nnxt->size_and_flag.prev_chunk_size += h->size_and_flag.current_chunk_size;
        rm_chunk_from_bin(nxt);
        bin[search_enbin(nxt->size_and_flag.current_chunk_size)]->size--;
        h->size_and_flag.current_chunk_size += nxt->size_and_flag.current_chunk_size;
        nxt->size_and_flag.current_chunk_size = 0;
    }
    if (h->size_and_flag.allocated_flag == True) {
        chunk_header_t *nxt = (chunk_header_t *)((intptr_t)(void*)h + (intptr_t)((chunk_header_t *)h)->size_and_flag.current_chunk_size);
        chunk_header_t *pre = (chunk_header_t *)((intptr_t)(void*)h - (intptr_t)((chunk_header_t *)h)->size_and_flag.prev_chunk_size);
        nxt->size_and_flag.prev_chunk_size += pre->size_and_flag.current_chunk_size;
        rm_chunk_from_bin(pre);
        bin[search_enbin(pre->size_and_flag.current_chunk_size)]->size--;
        pre->size_and_flag.current_chunk_size += h->size_and_flag.current_chunk_size;
        h->size_and_flag.current_chunk_size = 0;
        h->prev = NULL;
        h->next = NULL;
        return pre;
    } else {
        h->prev = NULL;
        h->next = NULL;
        return h;
    }
}

ll best_fit_size(ll bytes)
{
    ll size = bytes + 24;
    // for mmap
    if (bytes > (int)pow(2.0, 15)) {
        return size;
    }
    // for sbrk
    else {
        if (size >= (int)pow(2.0, 14)) size = (int)pow(2.0, 15);
        else if (size >= (int)pow(2.0, 13)) size = (int)pow(2.0, 14);
        else if (size >= (int)pow(2.0, 12)) size = (int)pow(2.0, 13);
        else if (size >= (int)pow(2.0, 11)) size = (int)pow(2.0, 21);
        else if (size >= (int)pow(2.0, 10)) size = (int)pow(2.0, 11);
        else if (size >= (int)pow(2.0, 9)) size = (int)pow(2.0, 10);
        else if (size >= (int)pow(2.0, 8)) size = (int)pow(2.0, 9);
        else if (size >= (int)pow(2.0, 7)) size = (int)pow(2.0, 8);
        else if (size >= (int)pow(2.0, 6)) size = (int)pow(2.0, 7);
        else if (size >= (int)pow(2.0, 5)) size = (int)pow(2.0, 6);
        else if (size >= (int)pow(2.0, 4)) size = (int)pow(2.0, 5);
        return size;
    }
}

void show_bin(int i)
{
    if (is_init == False) {
        return;
    }
    chunk_header_t *cur = bin[i]->next;
    while ((void *)cur != (void *)bin[i]) {
        void *r_cur = (void *)((intptr_t)(void*)cur - (intptr_t)(void*)get_start_sbrk());
        printf("0x%012p--------%d\n", (void *)(uintptr_t)r_cur, cur->size_and_flag.current_chunk_size);
        cur = cur->next;
    }
}

void show_all_bin()
{
    int i=0;
    for (i=0; i<BIN_SIZE; i++) show_bin(i);
}

int search_debin(ll size)
{
    for (int i = 0; i < BIN_SIZE; i++) {
        if (bin[i]->size == 0) {
            continue;
        }
        if (size <= (int)pow(2.0, i+5)) {
            if (size < (int)pow(2.0, i+5)) {
                //printf("bin[%d]->size = %d | size = %d\n", i, bin[i]->size, size);
                chunk_header_t *s = de_bin(i, size);
                chunk_header_t *c = split(&s, size);
                //show_all_bin();
                i=0;
                continue;
            }
            //printf("search_debin = %d\n", i);
            return i;
        }
    }
    printf("not any free chunk\n");
    return -1;
}

int search_enbin(ll size)
{
    int i=0;
    while(size /= 2) i++;
    return i - 5;
}

void en_bin(int index, chunk_header_t *c_h)
{
    if (bin[index]->size == 0) {
        bin[index]->next = c_h;
        c_h->prev = bin[index];
        bin[index]->prev = c_h;
        c_h->next = bin[index];
    } else {
        chunk_header_t *tmp;
        if (index < BIN_SIZE - 1) {
            tmp = bin[index]->prev;
            bin[index]->prev = c_h;
            c_h->next = bin[index];
            tmp->next = c_h;
            c_h->prev = tmp;
        }
    }
    bin[index]->size++;
}

chunk_header_t *de_bin(int index, ll size)
{
    if (bin[index]->size == 0) {
        return NULL;
    } else {
        chunk_header_t *ret;
        if (index < BIN_SIZE) {
            ret = bin[index]->next;
            rm_chunk_from_bin(ret);
            bin[index]->size--;
            return ret;
        }
        printf("In de_bin : de_bin fail = 0\n");
        return NULL;
    }
}

void rm_chunk_from_bin(chunk_header_t *c)
{
    /*Used to reconnect linked list when removing a chunk*/
    if (c->prev == bin[0] || c->prev == bin[1] || c->prev == bin[2] ||
            c->prev == bin[3] || c->prev == bin[4] || c->prev == bin[5] ||
            c->prev == bin[6] || c->prev == bin[7] || c->prev == bin[8] ||
            c->prev == bin[9] || c->prev == bin[10]) {
        ((bin_t *)c->prev)->next = c->next;
    } else {
        ((chunk_header_t *)c->prev)->next = c->next;
    }
    if (c->next == bin[0] || c->next == bin[1] || c->next == bin[2] ||
            c->next == bin[3] || c->next == bin[4] || c->next == bin[5] ||
            c->next == bin[6] || c->next == bin[7] || c->next == bin[8] ||
            c->next == bin[9] || c->next == bin[10]) {
        ((bin_t *)c->next)->prev = c->prev;
    } else {
        ((chunk_header_t *)c->next)->prev = c->prev;
    }
    c->prev = NULL;
    c->next = NULL;
}

int check_valid_free(void *a_mem)
{
    chunk_header_t *cur = get_start_sbrk();
    int count = 0;
    while (count++ < num_chunk + 1) {
        if ((intptr_t)(void*)cur > (intptr_t)(void*)a_mem - 24) {
            return False;
        }
        if (cur == a_mem - 24) {
            void *nxt;
            nxt = (void *)((intptr_t)(void*)cur + (intptr_t)cur->size_and_flag.current_chunk_size);
            if ((intptr_t)(void*)nxt - (intptr_t)(void*)get_start_sbrk() <= (int)pow(2.0, 15) &&
                    ((chunk_header_t *)nxt)->size_and_flag.allocated_flag == False) {
                return True;
            } else {
                return False;
            }
        }
        cur = (void *)((intptr_t)(void*)cur + (intptr_t)(cur->size_and_flag.current_chunk_size));
    }
    return 0;
}
