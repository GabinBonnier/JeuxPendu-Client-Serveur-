#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Struct with the informations of a game */
typedef struct {
    char secret_word[50];
    int nb_letters;
    int nb_life;
} Game;

/* Set a game */
void init_game(Game *g, char *word_choose);

/* Test the intput of the user */
void test_input_game(const Game *g, const char *input, int tab_occ[50]);

/* Show the life */
void affichage(const int nb_life);

#endif