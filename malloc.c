#include "malloc.h"

uint8_t* heap_start(){
    return MY_HEAP + sizeof(uint16_t);
}

uint8_t* heap_end(){
    return MY_HEAP + SIZE_HEAP;
}

EndBlock* end_of_block(BeginBlock* b) {
    return (EndBlock*)((uint8_t*)b + sizeof(BeginBlock) + b->size);
}

void set_end_block(BeginBlock* block) {
    EndBlock* end_block = end_of_block(block);
    if ((uint8_t*)end_block >= heap_start() && (uint8_t*)end_block + sizeof(EndBlock) <= heap_end()) {
        end_block->size = block->size;
    }
}

int in_heap(BeginBlock* block) {
    if (block == NULL) return 0;
    uint8_t* bstart = (uint8_t*)block;
    if (bstart < heap_start()) return 0;

    // verify that at least the header fits in the heap
    if (bstart + sizeof(BeginBlock) > heap_end()) return 0;
    return 1;
}

BeginBlock* get_prev_block(BeginBlock* block) {
    if (!in_heap(block)) return NULL;
    if ((uint8_t*)block == heap_start()) return NULL;
    
    // verify the end of the previous block
    EndBlock* end_Prev = (EndBlock*)((uint8_t*)block - sizeof(EndBlock));
    if ((uint8_t*)end_Prev < heap_start() || (uint8_t*)end_Prev + sizeof(EndBlock) > heap_end()) {
        return NULL;
    }

    // verify the size of the previous block
    size_t prev_Size = end_Prev->size;
    if (prev_Size == 0 || prev_Size > SIZE_HEAP) return NULL;

    // verify the address of the previous block, if it's in the range
    uint8_t* prev_Addr = (uint8_t*)end_Prev - sizeof(BeginBlock) - prev_Size;
    if (prev_Addr < heap_start()) return NULL;

    BeginBlock* prev = (BeginBlock*)prev_Addr;
    if (!in_heap(prev)) return NULL;

    EndBlock* check_End = end_of_block(prev);
    if (check_End != end_Prev) return NULL;

    return prev;
}

BeginBlock* get_next_block(BeginBlock* block) {
    if (!in_heap(block)) return NULL;

    EndBlock* end = end_of_block(block);

    // the next BeginBlock starts after the EndBlock
    uint8_t* next_begin = (uint8_t*)end + sizeof(EndBlock);
    if (next_begin + sizeof(BeginBlock) > heap_end()) return NULL;

    BeginBlock* next = (BeginBlock*)next_begin;
    if (!in_heap(next)) return NULL;

    // verify that the end of next is still in the heap
    EndBlock* nextEnd = end_of_block(next);
    if ((uint8_t*)nextEnd + sizeof(EndBlock) > heap_end()) return NULL;

    return next;
}

BeginBlock* get_prev_free(BeginBlock* block){
    if (!in_heap(block)) return NULL;

    // if block is at the beginning, there is no prev
    if ((uint8_t*)block == heap_start()) return NULL;

    BeginBlock* prev = block;

    while (1) {
        EndBlock* end_prev = (EndBlock*)((uint8_t*)prev - sizeof(EndBlock));
        if ((uint8_t*)end_prev < heap_start() || (uint8_t*)end_prev + sizeof(EndBlock) > heap_end()) {
            return NULL;
        }

        // address of the beginning of the previous block
        uint8_t* prev_addr = (uint8_t*)end_prev - sizeof(BeginBlock) - end_prev->size;
        if (prev_addr < heap_start()) return NULL;

        // the previous block
        prev = (BeginBlock*)prev_addr;
        if (!in_heap(prev)) return NULL;

        // verify that the end of prev is end_prev to be sure
        EndBlock* chk = end_of_block(prev);
        if (chk != end_prev) return NULL;
        if (prev->next != 0) return prev;
        
        if ((uint8_t*)prev == heap_start()) return NULL;
    }
}

BeginBlock* get_next_free(BeginBlock* block){
    if (!in_heap(block)) return NULL;

    BeginBlock* next = block;
    while (1) {
        // calculate address of the next logical block
        uint8_t* next_addr = (uint8_t*)next + sizeof(BeginBlock) + next->size + sizeof(EndBlock);
        if (next_addr + sizeof(BeginBlock) > heap_end()) return NULL;
        next = (BeginBlock*)next_addr;
        if (!in_heap(next)) return NULL;
        
        // verify that the end of next is still in the heap
        EndBlock* nextEnd = end_of_block(next);
        if ((uint8_t*)nextEnd + sizeof(EndBlock) > heap_end()) return NULL;
        if (next->next != 0) return next;
    }
}

uint16_t get_index(BeginBlock* block){
    uint8_t *addr = (uint8_t*) block;
    uint16_t index = addr - MY_HEAP;
    return index;
}

uint16_t read_2bytes(uint16_t index) {
    uint16_t valeur = MY_HEAP[index] | (MY_HEAP[index+1] << 8);
    return valeur;
}

void ecrire_d(uint16_t index, uint16_t valeur) {
    MY_HEAP[index]   = valeur & 0xFF;
    MY_HEAP[index+1] = (valeur >> 8) & 0xFF;
}

/* return the first free block of the heap */
BeginBlock* get_first_free() {
    return (BeginBlock*) &MY_HEAP[read_2bytes(0)];
}

void init(){
    // Reserve space at the beginning to store the address of the first free block
    ecrire_d(0, 2);
    
    // The first block starts after this reserved space
    BeginBlock* block = (BeginBlock*)heap_start();
    block->size = SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock) - 2;
    block->next = CODE_ERREUR;
    set_end_block(block);
}

void my_free(void *pointer) {
    if (!pointer) return;
    
    BeginBlock* block = ((BeginBlock*)pointer) - 1;
    if (!in_heap(block)) return;
    
    block->next = 0;
    set_end_block(block);
    
    // Merge with the previous block if free
    BeginBlock* prev = get_prev_block(block);
    if (prev && prev->next != 0) {
        prev->size = prev->size + block->size + sizeof(BeginBlock) + sizeof(EndBlock);
        set_end_block(prev);
        block = prev;
    }
    
    // Merge with the next block if free
    BeginBlock* next = get_next_block(block);
    if (next && next->next != 0) {
        block->size = block->size + next->size + sizeof(BeginBlock) + sizeof(EndBlock);
        set_end_block(block);
    }

    // reinsert the freed block into the linked list of free blocks, at the correct position
    BeginBlock* next_free = get_next_free(block);
    if (next_free != NULL){
        uint16_t index = get_index(next_free);
        block->next = index;
    }
    else block->next = CODE_ERREUR;

    prev = get_prev_free(block);
    uint16_t index = get_index(block);
    if (prev != NULL) prev->next = index;
    
    // update the beginning of the linked list.
    if (index < read_2bytes(0)) ecrire_d(0, index);
}

void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (size > SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock)) return NULL;

    BeginBlock* first_free = get_first_free();
    if (first_free == NULL) return NULL;

    // if the block indicated by first_free is not free, then search for the 1st free block.
    if (first_free->next == 0) {
        ecrire_d(0, get_index(get_next_free((BeginBlock*)heap_start())));
        first_free = get_first_free();
        if (first_free == NULL) return NULL;
    }

    BeginBlock* block = first_free;
    BeginBlock* prev_free = NULL; // store the previous free block;
    BeginBlock* best_fit = NULL; // store the smallest block whose size is greater than size + 7
    BeginBlock* prev_fit = NULL; // store the free block preceding the best fit block

    // Search for the best block
    while (block != NULL) {
        if (!in_heap(block)) break;

        if (block->next != 0 && block->size >= size) {
            size_t old_size = block->size;

            if (old_size >= size && old_size <= size+6){
                // Exact allocation, no division possible
                if (prev_free != NULL) prev_free->next = block->next;
                else ecrire_d(0,block->next);
                block->next = 0;
                return (void*)(block + 1);
            }

            // Check if we can split the block
            else if (old_size <= size + ((size*10)/100) || old_size >= size*10)
            {
                best_fit = block;
                prev_fit = prev_free;
                break;
            }else{
                // Search for the best block
                if (best_fit == NULL || block->size < best_fit->size) {
                    best_fit = block;
                    prev_fit = prev_free;
                }
            }
        }
        prev_free = block;
        if (block->next == CODE_ERREUR) break;
        block = (BeginBlock*) &MY_HEAP[block->next];
    }

    // If we found a best_fit, we split it
    if (best_fit != NULL) {
        size_t old_size = best_fit->size;
        best_fit->size = size;
        set_end_block(best_fit);
        
        // Create a new free block after best_fit
        BeginBlock* next = (BeginBlock*)((uint8_t*)best_fit + sizeof(BeginBlock) + size + sizeof(EndBlock));
        
        if ((uint8_t*)next + sizeof(BeginBlock) + sizeof(EndBlock) <= heap_end()) {
            next->size = old_size - size - sizeof(BeginBlock) - sizeof(EndBlock);
            next->next = best_fit->next;
            set_end_block(next);
            
            if (prev_fit != NULL) prev_fit->next = get_index(next);
            else ecrire_d(0, get_index(next));
        } else {
            if (prev_fit != NULL) prev_fit->next = best_fit->next;
            else ecrire_d(0, best_fit->next);
        }
        best_fit->next = 0;
        return (void*)(best_fit + 1);
    }
    
    return NULL;
}