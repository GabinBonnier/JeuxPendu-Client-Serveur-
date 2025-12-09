#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "game.h"

/* Initialise une partie */
void init_game(Game *g) {

    // Tab of possible word
    static const char* word_possible[] = {
        "mot",
        "truc",
        "ligne",
        "casque",
        "trompette"
    };
    
    //Reset the aleatory
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }

    //Aleatory
    int count = sizeof(word_possible) / sizeof(word_possible[0]);
    int nb_choose = rand() % count;

    strcpy(g->secret_word, word_possible[nb_choose]);
    g->nb_letters = strlen(g->secret_word);
}


// Test the intput of 
void test_input_game(const Game *g, const char *input, int tab_occ[50]) {

    const char *secret_word = g->secret_word;

    //Intialise the results
    int len_input = strlen(input);
    for (int i = 0; i < 50; i++) {
        tab_occ[i] = -1;
    }
    int count = 1;

    // For a letter
    if (len_input == 1) {
        char letter = input[0];

        for (int i = 0; secret_word[i] != '\0'; i++) {
            if (secret_word[i] == letter) {
                tab_occ[count++] = i;
            }
        }

        // Not find
        if (count == 1)
            tab_occ[0] = -1;
        else
            tab_occ[0] = count - 1;

        return;
    }

    //For a word
    if (strcmp(secret_word, input) == 0)
        tab_occ[0] = 100;
    else
        tab_occ[0] = -1;
}