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

    printf("Etat de la mémoire àpres les tests : \n");
    printf("Nombre de blocs alloués : %d\n", mallocs);
    printf("Taille totale de blocs alloués : %d\n", total_malloc_size);
    printf("Le plus gros bloc alloué : %d\n", bigest_malloc_size);
    printf("Nombre de blocs libres : %d\n", frees);
    printf("Taille totale de blocs libres : %d\n", total_free_size);
    printf("Le plus gros bloc libre : %d\n", bigest_free_size);
    
}

void test_random_steps(){
    init();

    //déterminer un range de taille pour les données à allouer
    uint16_t sizemax = 1000;
    uint16_t sizemin = 500;
    uint16_t length = SIZE_HEAP/sizemin;

    //compteurs de résultats : 
    int nulls = 0;
    int mallocs = 0;
    int frees = 0;

    // créer un tableau qui contient tous les pointeurs vers les zones allouées
    void* tab[length];
    for (uint16_t i=0; i<length; i++){
        tab[i] = NULL;
    }

    for (int step=0; step<1000; step++){
        uint16_t i = rand() % length; // choisir une case aléatoire
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
  
    printf("Résultats du test random_steps : \n");
    printf("Nombre de mallocs : %d\n", mallocs);
    printf("Nombre de frees : %d\n", frees);
    printf("Nombre de nuls : %d\n", nulls);
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
    
    printf("Résultats du test random_time : \n");
    printf("Nombre de mallocs : %d\n", mallocs);
    printf("Nombre de frees : %d\n", frees);
    printf("Nombre de nuls : %d\n", nulls);
    printf("Temps écoulé : %.2f secondes\n", elapsed);
    printf("\n");
    etat_memoire();
    printf("\n");
}

int main(void) {
    test_random_steps();
    test_random_time();
    
    
    printf("Tests terminés! 🎉\n");
    return 0;
}
