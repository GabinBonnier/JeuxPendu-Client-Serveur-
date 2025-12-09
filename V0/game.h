#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Structure contenant l'état du jeu */
typedef struct {
    char secret_word[50];
    int nb_letters;
} Game;

/* Initialise une partie*/
void init_game(Game *g);

/* Teste une entrée (lettre ou mot)*/
void test_input_game(const Game *g, const char *input, int tab_occ[50]);

#endif