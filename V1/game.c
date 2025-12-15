#include <stdio.h>
#include <string.h>
#include "game.h"

/* Starts a game with a given word */
void init_game(Game *g, const char *word) {
    strncpy(g->secret_word, word, MAX_WORD_LEN);
    g->secret_word[MAX_WORD_LEN-1] = '\0';
    g->nb_letters = strlen(g->secret_word);
    g->nb_life = 6;
}

/* Tests user input */
void test_input_game(const Game *g, const char *input, int tab_occ[50]) {
    const char *secret_word = g->secret_word;
    int len_input = strlen(input);
    for (int i = 0; i < 50; i++) tab_occ[i] = -1;
    int count = 1;

    if (len_input == 1) {
        char letter = input[0];
        for (int i = 0; secret_word[i] != '\0'; i++) {
            if (secret_word[i] == letter) tab_occ[count++] = i;
        }
        tab_occ[0] = (count == 1) ? -1 : count - 1;
        return;
    }

    tab_occ[0] = (strcmp(secret_word, input) == 0) ? 100 : -1;
}

/* Display of the hanged man */
void affichage(const int nb_life) {    
    switch(nb_life) {
        case 6:
            printf("  +---+\n  |   |\n      |\n      |\n      |\n      |\n=========\n"); break;
        case 5:
            printf("  +---+\n  |   |\n  O   |\n      |\n      |\n      |\n=========\n"); break;
        case 4:
            printf("  +---+\n  |   |\n  O   |\n  |   |\n      |\n      |\n=========\n"); break;
        case 3:
            printf("  +---+\n  |   |\n  O   |\n /|   |\n      |\n      |\n=========\n"); break;
        case 2:
            printf("  +---+\n  |   |\n  O   |\n /|\\  |\n      |\n      |\n=========\n"); break;
        case 1:
            printf("  +---+\n  |   |\n  O   |\n /|\\  |\n /    |\n      |\n=========\n"); break;
        case 0:
            printf("  +---+\n  |   |\n  O   |\n /|\\  |\n / \\  |\n      |\n=========\n"); break;
    }
}
