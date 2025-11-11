#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include "malloc.h"

uint8_t MY_HEAP[SIZE_HEAP];

void etat_memoire(){
    int frees = 0;
    int mallocs = 0;
    int total_free_size = 0;
    int total_malloc_size = 0;
    int bigest_free_size = 0;
    int bigest_malloc_size = 0;

    BeginBlock* block = (BeginBlock*) heap_start();
    while ( block != NULL && in_heap(block))
    {
        if (block->next == 0) {
            mallocs++;
            total_malloc_size += block->size;
            if (block->size > bigest_malloc_size) bigest_malloc_size = block->size;
        }
        else {
            frees++;
            total_free_size += block->size;
            if (block->size > bigest_free_size) bigest_free_size = block->size;
        }
        block = get_next_block(block);
    }

    printf("Memory state after tests: \n");
    printf("Number of allocated blocks: %d\n", mallocs);
    printf("Total size of allocated blocks: %d\n", total_malloc_size);
    printf("Largest allocated block: %d\n", bigest_malloc_size);
    printf("Number of free blocks: %d\n", frees);
    printf("Total size of free blocks: %d\n", total_free_size);
    printf("Largest free block: %d\n", bigest_free_size);
    
}

void test_random_steps(){
    init();

    // determine a size range for the data to allocate
    uint16_t sizemax = 1000;
    uint16_t sizemin = 500;
    uint16_t length = SIZE_HEAP/sizemin;

    // result counters: 
    int nulls = 0;
    int mallocs = 0;
    int frees = 0;

    // create an array that contains all pointers to allocated zones
    void* tab[length];
    for (uint16_t i=0; i<length; i++){
        tab[i] = NULL;
    }

    for (int step=0; step<1000; step++){
        uint16_t i = rand() % length; // choose a random slot
        if (tab[i]==NULL){
            size_t size = sizemin + rand() % (sizemax - sizemin + 1);
            tab[i] = my_malloc(size);
            if (tab[i]==NULL){
                nulls++;
            }
            
            else {
                mallocs++;
            }
        }
        else {
            my_free(tab[i]);
            tab[i] = NULL;
            frees++;
        }
    }
  
    printf("Results of random_steps test: \n");
    printf("Number of mallocs: %d\n", mallocs);
    printf("Number of frees: %d\n", frees);
    printf("Number of nulls: %d\n", nulls);
    printf("\n");
    etat_memoire();
    printf("\n");
}

void test_random_time(){
    init();

    uint16_t sizemax = 1000;
    uint16_t sizemin = 500;
    uint16_t length = SIZE_HEAP/sizemin;

    int nulls = 0;
    int mallocs = 0;
    int frees = 0;

    void* tab[length];
    for (uint16_t i=0; i<length; i++){
        tab[i] = NULL;
    }

    struct timeval start, now;
    gettimeofday(&start, NULL);

    double elapsed = 0.0;
    while (elapsed < 1.0) {
        uint16_t i = rand() % length;
        if (tab[i]==NULL){
            size_t size = sizemin + rand() % (sizemax - sizemin + 1);
            tab[i] = my_malloc(size);
            if (tab[i]==NULL){
                nulls++;
            } else {
                mallocs++;
            }
        } else {
            my_free(tab[i]);
            tab[i] = NULL;
            frees++;
        }
        gettimeofday(&now, NULL);
        elapsed = (now.tv_sec - start.tv_sec) + (now.tv_usec - start.tv_usec) / 1e6;
    }
    
    printf("Results of random_time test: \n");
    printf("Number of mallocs: %d\n", mallocs);
    printf("Number of frees: %d\n", frees);
    printf("Number of nulls: %d\n", nulls);
    printf("Elapsed time: %.2f seconds\n", elapsed);
    printf("\n");
    etat_memoire();
    printf("\n");
}

int main(void) {
    test_random_steps();
    test_random_time();
    
    
    printf("Tests completed! 🎉\n");
    return 0;
}
