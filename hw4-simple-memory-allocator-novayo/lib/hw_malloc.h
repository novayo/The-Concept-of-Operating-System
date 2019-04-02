#ifndef HW_MALLOC_H
#define HW_MALLOC_H
#define ll long long int
#include <unistd.h>

typedef void *chunk_ptr_t; //8bytes
void *hw_malloc(size_t bytes);
int hw_free(void *mem);
void *get_start_sbrk(void);

ll best_fit_size(ll bytes);
void show_bin(int i);
void show_all_bin();


typedef enum {
    False = 0,
    True = 1,
} Bool;

typedef struct struct_chunk_info_t { //64bits = 8bytes
    ll current_chunk_size : 31;
    ll prev_chunk_size : 31;
    Bool allocated_flag : 1;
    Bool mmap_flag : 1;
} chunk_info_t;

typedef struct struct_chunk_header { //24bytes
    chunk_ptr_t prev;
    chunk_ptr_t next;
    chunk_info_t size_and_flag;
} chunk_header_t;

typedef struct bin_t {
    chunk_ptr_t prev;
    chunk_ptr_t next;
    int size;
} bin_t;

#endif
