#ifndef MALLOC_H
#define MALLOC_H

#include <stdio.h>
#include <stdint.h>

#define SIZE_HEAP 64000 //la taille maximale de la heap
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
uint8_t* heap_start();


/* return la fin de la heap*/
uint8_t* heap_end();


/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * return: la fin d'un bloc (adresse du EndBlock)
 * */
EndBlock* end_of_block(BeginBlock*);


/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * effet: écrit au bon endroit les méta données de la fin du bloc
 */
void set_end_block(BeginBlock*);


/** 
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * return:
 *  - 1 regarde si le block se trouve dans le range de la heap
 *  - 0 sinon
 */
int in_heap(BeginBlock*);


/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 * return:
 *  - le block précédent, libre ou occupé, s'il existe
 *  - NULL si il n'y a pas des blocs précédent ou si l'adresse passée en argument ne se trouve pas dans la heap.
 * */
BeginBlock* get_prev_block(BeginBlock*);


/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 *  return:
 *  - le block suivant, libre ou occupé, s'il existe
 *  - NULL si dépassement de la heap ou si l'adresse du bloc en argument ne se trouve pas dans la heap
 * */
BeginBlock* get_next_block(BeginBlock*);


/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 *  return:
 *  - le block libre précédent, s'il existe
 *  - NULL si dépassement de la heap ou si l'adresse du bloc en argument ne se trouve pas dans la heap
 * */
BeginBlock* get_prev_free(BeginBlock*);


/**
 * args: BeginBlock*, les métadonnées du début de bloc
 * 
 *  return:
 *  - le block libre suivant, s'il existe
 *  - NULL si dépassement de la heap ou si l'adresse du bloc en argument ne se trouve pas dans la heap
 * */
BeginBlock* get_next_free(BeginBlock*);


/**
 * args : BeginBlock* les métadonnées du début du block
 * 
 * return : l'indice de du bloc dans la heap 
 */
uint16_t get_index(BeginBlock*);


/**
 * args : uint16_t un indice du tableau
 * 
 * return : les deux octets de métadonnées situés à droite de l'indice, transformés en une valeur décimale
 */
uint16_t read_2bytes(uint16_t);


/**
 * args : uint16_t un indice du tableau
 *         uint16_t la valeur qu'on veut écrire
 * 
 * effet : écrit 16 bits de données dans la heap à partir de l'index donné
 */ 
void write_2bytes(uint16_t, uint16_t);


/* return le premier bloc libre de la heap */
BeginBlock* get_first_free();


/** 
 * La fonction init initialise la mémoire en plaçant un bloc de métadonnées au début et à la fin du tableau 
 * le bloc commence apres 2 bytes réservés pour stocker first_free, l'indice du premier bloc libre.
 * On y inscrit 2, l'indice du début du reste de la mémoire et donc du premier bloc libre.
 * */
void init();


/**
 * La fonciton my_free reçoit un pointeur vers une zone du tableau. 
 * Elle vérifie à gauche et à droite du segment à libérer pour éventuellement fusionner des zones libres.
 * Elle insère le bloc libéré dans la liste chaînée des blocs libres, au bon endroit
 * Elle met à jour l'indice du premier bloc libre au debut de la heap si nécessaire 
 */
void my_free(void *);


/**
 * La fonction my_malloc reçoit une taille 'size' en argument qui indique le nombre d'octets dont l'utilisateur a besoin.
 * Elle trouve un emplacement adéquat pour glisser le segment demandé, avec ses 6 blocs de métadonnées.
 * Elle modifie (si nécessaire) les métadonnées du bloc vide restant après l'allocation de mémoire.
 * Elle retourne à l'utilisateur un pointeur vers l'adresse du début de son segment de données utiles.
 * 
 * Si la taille du bloc demandé dépasse la taille de la mémoire ou si la mémoire est trop fragmentée pour placer le bloc, 
 * la fonction retourne NULL.  
 */
void *my_malloc(size_t);

#endif