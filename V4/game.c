#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "game.h"

/* Initialise une partie */
void init_game(Game *g, char *word_choose) {

    // Initalize secret word
    strcpy(g->secret_word, word_choose);
    g->nb_letters = strlen(g->secret_word);

    //Set the nb_life
    g->nb_life = 6;
}


// Test the intput of the user
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


/* Show the life */
void affichage(const int nb_life) {    

    switch(nb_life) {
        case 6:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 5:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("  O   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 4:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("  O   |\n");
            printf("  |   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 3:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|   |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 2:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|\\  |\n");
            printf("      |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 1:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|\\  |\n");
            printf(" /    |\n");
            printf("      |\n");
            printf("=========\n");
            break;
        case 0:
            printf("Vies restantes : %d\n", nb_life);
            printf("  |   |\n");
            printf("  O   |\n");
            printf(" /|\\  |\n");
            printf(" / \\  |\n");
            printf("      |\n");
            printf("=========\n");
            break;
    }
}