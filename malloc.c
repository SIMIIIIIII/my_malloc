#include <stdio.h>
#include <stdint.h>

#define SIZE_HEAP 64000
#define CODE_ERREUR 65535
extern uint8_t MY_HEAP[SIZE_HEAP];

typedef struct BeginBlock{
    uint16_t size;
    uint16_t next; //si libre, indice du prochain bloc libre ou 65535 si c'est le dernier bloc libre, sinon 0;
} BeginBlock;

typedef struct{
    uint16_t size;
} EndBlock;

/* return le debut de la heap */
uint8_t* heap_start(){
    return MY_HEAP + sizeof(uint16_t);
}

/* return la fin de la heap*/
uint8_t* heap_end(){
    return MY_HEAP + SIZE_HEAP;
}

/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * return: la fin d'un bloc (adresse du EndBlock)
 * */
EndBlock* end_of_block(BeginBlock* b) {
    return (EndBlock*)((uint8_t*)b + sizeof(BeginBlock) + b->size);
}

/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * effet: écrit au bon endroit les méta données de la fin du bloc
 */
void set_end_block(BeginBlock* block) {
    EndBlock* end_block = end_of_block(block);
    if ((uint8_t*)end_block >= heap_start() && (uint8_t*)end_block + sizeof(EndBlock) <= heap_end()) {
        end_block->size = block->size;
    }
}

/** 
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * return:
 *  - 1 regarde si le block se trouve dans le range de la heap
 *  - 0 sinon
 */
int in_heap(BeginBlock* block) {
    if (block == NULL) return 0;
    uint8_t* bstart = (uint8_t*)block;
    if (bstart < heap_start()) return 0;

    // vérifier qu'au moins le header tient dans la heap
    if (bstart + sizeof(BeginBlock) > heap_end()) return 0;
    return 1;
}

/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * return:
 *  - le block précédent, libre ou occupé, s'il existe
 *  - NULL si il n'y a pas des blocs précédent ou si l'adresse passée en argument ne se trouve pas dans la heap.
 * */
BeginBlock* get_prev_block(BeginBlock* block) {
    if (!in_heap(block)) return NULL;
    if ((uint8_t*)block == heap_start()) return NULL;
    
    //verifier la fin de la liste précedente
    EndBlock* end_Prev = (EndBlock*)((uint8_t*)block - sizeof(EndBlock));
    if ((uint8_t*)end_Prev < heap_start() || (uint8_t*)end_Prev + sizeof(EndBlock) > heap_end()) {
        return NULL;
    }

    //verifier la taille de la liste précedente
    size_t prev_Size = end_Prev->size;
    if (prev_Size == 0 || prev_Size > SIZE_HEAP) return NULL;

    //vérifier l'adresse de la liste précédente, si elle se trouve dans le range
    uint8_t* prev_Addr = (uint8_t*)end_Prev - sizeof(BeginBlock) - prev_Size;
    if (prev_Addr < heap_start()) return NULL;

    BeginBlock* prev = (BeginBlock*)prev_Addr;
    if (!in_heap(prev)) return NULL;

    EndBlock* check_End = end_of_block(prev);
    if (check_End != end_Prev) return NULL;

    return prev;
}

/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 *  return:
 *  - le block suivant, libre ou occupé, s'il existe
 *  - NULL si dépassement de la heap ou si l'adresse du bloc en argument ne se trouve pas dans la heap
 * */
BeginBlock* get_next_block(BeginBlock* block) {
    if (!in_heap(block)) return NULL;

    EndBlock* end = end_of_block(block);

    // le BeginBlock suivant commence après la EndBlock
    uint8_t* next_begin = (uint8_t*)end + sizeof(EndBlock);
    if (next_begin + sizeof(BeginBlock) > heap_end()) return NULL;

    BeginBlock* next = (BeginBlock*)next_begin;
    if (!in_heap(next)) return NULL;

    // verifier que le end du next est toujours dans la heap
    EndBlock* nextEnd = end_of_block(next);
    if ((uint8_t*)nextEnd + sizeof(EndBlock) > heap_end()) return NULL;

    return next;
}

/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 *  return:
 *  - le block libre précédent, s'il existe
 *  - NULL si dépassement de la heap ou si l'adresse du bloc en argument ne se trouve pas dans la heap
 * */
BeginBlock* get_prev_free(BeginBlock* block){
    if (!in_heap(block)) return NULL;

    // si block est au début, il n'y a pas de prev
    if ((uint8_t*)block == heap_start()) return NULL;

    BeginBlock* prev = block;

    while (1) {
        EndBlock* end_prev = (EndBlock*)((uint8_t*)prev - sizeof(EndBlock));
        if ((uint8_t*)end_prev < heap_start() || (uint8_t*)end_prev + sizeof(EndBlock) > heap_end()) {
            return NULL;
        }

        // adresse du début du block précédent
        uint8_t* prev_addr = (uint8_t*)end_prev - sizeof(BeginBlock) - end_prev->size;
        if (prev_addr < heap_start()) return NULL;

        // le bloc précédent
        prev = (BeginBlock*)prev_addr;
        if (!in_heap(prev)) return NULL;

        // vérifier que le end du prev est end_prev pour être sûr
        EndBlock* chk = end_of_block(prev);
        if (chk != end_prev) return NULL;
        if (prev->next != 0) return prev;
        
        if ((uint8_t*)prev == heap_start()) return NULL;
    }
}

/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 *  return:
 *  - le block libre suivant, s'il existe
 *  - NULL si dépassement de la heap ou si l'adresse du bloc en argument ne se trouve pas dans la heap
 * */
BeginBlock* get_next_free(BeginBlock* block){
    if (!in_heap(block)) return NULL;

    BeginBlock* next = block;
    while (1) {
        // calculer adresse du suivant logique
        uint8_t* next_addr = (uint8_t*)next + sizeof(BeginBlock) + next->size + sizeof(EndBlock);
        if (next_addr + sizeof(BeginBlock) > heap_end()) return NULL;
        next = (BeginBlock*)next_addr;
        if (!in_heap(next)) return NULL;
        
        // verifier que le end du next est toujours dans la heap
        EndBlock* nextEnd = end_of_block(next);
        if ((uint8_t*)nextEnd + sizeof(EndBlock) > heap_end()) return NULL;
        if (next->next != 0) return next;
    }
}

/**
 * args : BeginBlock* les métadonnées du début du block
 * 
 * return : l'indice de du bloc dans la heap 
 */
uint16_t get_index(BeginBlock* block){
    uint8_t *addr = (uint8_t*) block;
    uint16_t index = addr - MY_HEAP;
    return index;
}

/**
 * args : uint16_t un indice du tableau
 * 
 * return : les deux octets de métadonnées situés à droite de l'indice, transformés en une valeur décimale
 */
uint16_t read_2bytes(uint16_t index) {
    uint16_t valeur = MY_HEAP[index] | (MY_HEAP[index+1] << 8);
    return valeur;
}

/**
 * args : uint16_t un indice du tableau
 *         uint16_t la valeur qu'on veut écrire
 * 
 * effet : écrit 16 bits de données dans la heap à partir de l'index donné
 */ 
void ecrire_d(uint16_t index, uint16_t valeur) {
    MY_HEAP[index]   = valeur & 0xFF;
    MY_HEAP[index+1] = (valeur >> 8) & 0xFF;
}

/* return le premier bloc libre de la heap */
BeginBlock* get_first_free() {
    return (BeginBlock*) &MY_HEAP[read_2bytes(0)];
}

/** 
 * La fonction init initialise la mémoire en plaçant un bloc de métadonnées au début et à la fin du tableau 
 * le bloc commence apres 2 bytes réservés pour stocker first_free, l'indice du premier bloc libre.
 * On y inscrit 2, l'indice du début du reste de la mémoire et donc du premier bloc libre.
 * */
void init(){
    // Réserver de l'espace au début pour stocker l'adresse du premier block libre
    ecrire_d(0, 2);
    
    // Le premier bloc commence après cet espace réservé
    BeginBlock* block = (BeginBlock*)heap_start();
    block->size = SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock) - 2;
    block->next = CODE_ERREUR;
    set_end_block(block);
}

/**
 * La fonciton my_free reçoit un pointeur vers une zone du tableau. 
 * Elle vérifie à gauche et à droite du segment à libérer pour éventuellement fusionner des zones libres.
 * Elle insère le bloc libéré dans la liste chaînée des blocs libres, au bon endroit
 * Elle met à jour l'indice du premier bloc libre au debut de la heap si nécessaire 
 */
void my_free(void *pointer) {
    if (!pointer) return;
    
    BeginBlock* block = ((BeginBlock*)pointer) - 1;
    if (!in_heap(block)) return;
    
    block->next = 0;
    set_end_block(block);
    
    // Fusion avec le bloc précédent si libre
    BeginBlock* prev = get_prev_block(block);
    if (prev && prev->next != 0) {
        prev->size = prev->size + block->size + sizeof(BeginBlock) + sizeof(EndBlock);
        set_end_block(prev);
        block = prev;
    }
    
    // Fusion avec le bloc suivant si libre
    BeginBlock* next = get_next_block(block);
    if (next && next->next != 0) {
        block->size = block->size + next->size + sizeof(BeginBlock) + sizeof(EndBlock);
        set_end_block(block);
    }

    // réinsert le bloc libéré dans la liste chaînée des blocs libres, au bon endroit
    BeginBlock* next_free = get_next_free(block);
    if (next_free != NULL){
        uint16_t index = get_index(next_free);
        block->next = index;
    }
    else block->next = CODE_ERREUR;

    prev = get_prev_free(block);
    uint16_t index = get_index(block);
    if (prev != NULL) prev->next = index;
    
    // mettre à jour le debut de la liste chainée.
    if (index < read_2bytes(0)) ecrire_d(0, index);
}

/**
 * La fonction my_malloc reçoit une taille 'size' en argument qui indique le nombre d'octets dont l'utilisateur a besoin.
 * Elle trouve un emplacement adéquat pour glisser le segment demandé, avec ses 6 blocs de métadonnées.
 * Elle modifie (si nécessaire) les métadonnées du bloc vide restant après l'allocation de mémoire.
 * Elle retourne à l'utilisateur un pointeur vers l'adresse du début de son segment de données utiles.
 * 
 * Si la taille du bloc demandé dépasse la taille de la mémoire ou si la mémoire est trop fragmentée pour placer le bloc, 
 * la fonction retourne NULL.  
 */
void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    if (size > SIZE_HEAP - sizeof(BeginBlock) - sizeof(EndBlock)) return NULL;

    BeginBlock* first_free = get_first_free();
    if (first_free == NULL) return NULL;

    // si le bloc indiqué par first_free n'est pas libre, alors rechercher le 1er bloc libre.
    if (first_free->next == 0) {
        ecrire_d(0, get_index(get_next_free((BeginBlock*)heap_start())));
        first_free = get_first_free();
        if (first_free == NULL) return NULL;
    }

    BeginBlock* block = first_free;
    BeginBlock* prev_free = NULL; // stock le bloc libre précédent;
    BeginBlock* best_fit = NULL; // stock le plus petit bloc dont la taille est supperieur à size + 7
    BeginBlock* prev_fit = NULL; // stock le bloc libre pécendent le bloc best fit

    // Recherche du meilleur bloc
    while (block != NULL) {
        if (!in_heap(block)) break;

        if (block->next != 0 && block->size >= size) {
            size_t old_size = block->size;

            if (old_size >= size && old_size <= size+6){
                // Allocation exacte, pas de division possible
                if (prev_free != NULL) prev_free->next = block->next;
                else ecrire_d(0,block->next);
                block->next = 0;
                return (void*)(block + 1);
            }

            // Vérifier si on peut diviser le bloc
            else if (old_size <= size + ((size*10)/100) || old_size >= size*10)
            {
                best_fit = block;
                prev_fit = prev_free;
                break;
            }else{
                // Chercher le meilleur bloc
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

    // Si on a trouvé un best_fit, on le divise
    if (best_fit != NULL) {
        size_t old_size = best_fit->size;
        best_fit->size = size;
        set_end_block(best_fit);
        
        // Créer un nouveau bloc libre après best_fit
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