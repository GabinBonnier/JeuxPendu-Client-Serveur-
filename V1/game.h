#ifndef GAME_H
#define GAME_H

#define MAX_WORD_LEN 50

typedef struct {
    char secret_word[MAX_WORD_LEN];
    int nb_letters;
    int nb_life;
} Game;

void init_game(Game *g, const char *word);
void test_input_game(const Game *g, const char *input, int tab_occ[50]);
void affichage(const int nb_life);

#endif
