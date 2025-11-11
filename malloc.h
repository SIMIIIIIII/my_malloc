#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>
#include <stdint.h>

#define SIZE_HEAP 64000 // maximum heap size
#define CODE_ERREUR 65535
extern uint8_t MY_HEAP[SIZE_HEAP];


typedef struct BeginBlock{
    uint16_t size;
    uint16_t next; // if free, index of the next free block or 65535 if it's the last free block, otherwise 0;
} BeginBlock;


typedef struct{
    uint16_t size;
} EndBlock;


/* return the start of the heap */
uint8_t* heap_start();


/* return the end of the heap */
uint8_t* heap_end();


/**
 * args: BeginBlock*, the metadata of the block start
 * 
 * return: the end of a block (address of EndBlock)
 * */
EndBlock* end_of_block(BeginBlock*);


/**
 * args: BeginBlock*, the metadata of the block start
 * 
 * effect: writes the metadata of the block end at the correct location
 */
void set_end_block(BeginBlock*);


/** 
 * args: BeginBlock*, the metadata of the block start
 * 
 * return:
 *  - 1 if the block is within the heap range
 *  - 0 otherwise
 */
int in_heap(BeginBlock*);


/**
 * args: BeginBlock*, the metadata of the block start
 * 
 * return:
 *  - the previous block, free or occupied, if it exists
 *  - NULL if there is no previous block or if the address passed as argument is not in the heap.
 * */
BeginBlock* get_prev_block(BeginBlock*);


/**
 * args: BeginBlock*, the metadata of the block start
 * 
 *  return:
 *  - the next block, free or occupied, if it exists
 *  - NULL if heap overflow or if the block address in argument is not in the heap
 * */
BeginBlock* get_next_block(BeginBlock*);


/**
 * args: BeginBlock*, the metadata of the block start
 * 
 *  return:
 *  - the previous free block, if it exists
 *  - NULL if heap overflow or if the block address in argument is not in the heap
 * */
BeginBlock* get_prev_free(BeginBlock*);


/**
 * args: BeginBlock*, the metadata of the block start
 * 
 *  return:
 *  - the next free block, if it exists
 *  - NULL if heap overflow or if the block address in argument is not in the heap
 * */
BeginBlock* get_next_free(BeginBlock*);


/**
 * args : BeginBlock* the metadata of the block start
 * 
 * return : the index of the block in the heap 
 */
uint16_t get_index(BeginBlock*);


/**
 * args : uint16_t an array index
 * 
 * return : the two bytes of metadata located at the index, converted to a decimal value
 */
uint16_t read_2bytes(uint16_t);


/**
 * args : uint16_t an array index
 *         uint16_t the value to write
 * 
 * effect : writes 16 bits of data to the heap starting from the given index
 */ 
void write_2bytes(uint16_t, uint16_t);


/* return the first free block of the heap */
BeginBlock* get_first_free();


/** 
 * The init function initializes memory by placing a metadata block at the beginning and end of the array 
 * the block starts after 2 bytes reserved to store first_free, the index of the first free block.
 * We write 2 there, the index of the start of the rest of the memory and thus of the first free block.
 * */
void init();


/**
 * The my_free function receives a pointer to an array area. 
 * It checks left and right of the segment to be freed to potentially merge free zones.
 * It inserts the freed block into the linked list of free blocks, at the correct position
 * It updates the index of the first free block at the beginning of the heap if necessary 
 */
void my_free(void *);


/**
 * The my_malloc function receives a size 'size' as argument indicating the number of bytes the user needs.
 * It finds an adequate location to insert the requested segment, with its 6 metadata blocks.
 * It modifies (if necessary) the metadata of the remaining empty block after memory allocation.
 * It returns to the user a pointer to the address of the beginning of their useful data segment.
 * 
 * If the requested block size exceeds the memory size or if memory is too fragmented to place the block, 
 * the function returns NULL.  
 */
void *my_malloc(size_t);

#endif