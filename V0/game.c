#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

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

/*Test the intput :
- for a letter if it's on the secret word, each occurence is add to a tab
- for a word if it's the same as the secret word, tab[0] is 100
- if no occurence or word false, tab[0]=-1
return the tab
*/
void test_input(char *secret_word, const char *input, int tab_occ[50]) {

    //Value write by the user
    int len_input = strlen(input);
    int count = 0;

    //If the intput is only 1 -> a letter
    if (len_input == 1) {
        char letter = input[0];

        //Test if the lettre is in the word
        for (int i = 0; secret_word[i] != '\0'; i++) {
            if (secret_word[i] == letter) {
                tab_occ[count] = i;
                count++;
            }
        }

        if (count == 0)
            tab_occ[0] = -1;
    }

    //Else it's a word
    else {
        if (strcmp(secret_word, input) == 0)
            tab_occ[0] = 100;
        else
            tab_occ[0] = -1;
    }
}


int main(){
    char* word = def_word_secret();
    printf("%s\n",word);
    int tab[50];
    test_input(word,"l",tab);
    printf("%d\n",tab[0]);
    return 0;
}