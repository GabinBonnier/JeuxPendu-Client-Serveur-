#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*Choose the word for the game among a list of possible word */
char* def_word_secret(){

    //Tab of possibilities words
    const char* word_possible[] = {
        "mot",
        "truc",
        "ligne",
        "casque",
        "trompette"
    };

    //Choose the word using random
    int count = sizeof(word_possible) / sizeof(word_possible[0]);
    int nb_choose = rand() % count;

    return (char*) word_possible[nb_choose];
}

int main(){
    srand( time( NULL ) );
    printf("Mot secret défini");
    return 0;
}