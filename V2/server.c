#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5003
#define LG_MESSAGE 256

#include "game.h"

int main() {
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;

    //Create the listening socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if(socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée ! (%d)\n", socketEcoute);

    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //Initialize server address structure
    socklen_t longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = AF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    //Bind the socket to the specified IP and port
    if(bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0){
        perror("bind");
        exit(-2);
    }

    //Start listening for connections
    if(listen(socketEcoute, 5) < 0){
        perror("listen");
        exit(-3);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    //Infinite loop waiting for two clients
    while(1) {

        //Accept connection from player 1
        struct sockaddr_in addr1;
        socklen_t l1 = sizeof(addr1);
        int player1 = accept(socketEcoute, (struct sockaddr *)&addr1, &l1);
        if(player1 < 0) {
            perror("accept");
            continue;
        }
        printf("Joueur 1 connecté \n");

        //Accept connection from player 2
        struct sockaddr_in addr2;
        socklen_t l2 = sizeof(addr2);
        int player2 = accept(socketEcoute, (struct sockaddr *)&addr2, &l2);
        if(player2 < 0) {
            perror("accept");
            close(player1);
            continue;
        }
        printf("Joueur 2 connecté \n");

        //Initialize the game
        Game game;
        char motSecret[50];

        //Ask player 1 to provide the secret word
        send(player1, "choose", 7, 0);
        memset(motSecret, 0, sizeof(motSecret));
        recv(player1, motSecret, sizeof(motSecret), 0);
        motSecret[strcspn(motSecret, "\n")] = 0;
        init_game(&game, motSecret);

        char word_global[50];
        strcpy(word_global, game.secret_word);
        printf("Le mot comporte %d lettres.\n", (int)strlen(word_global));
        printf("Mot choisi : %s\n", word_global);

        //Send start message to both players
        char reponse[LG_MESSAGE];
        sprintf(reponse, "start %ld", strlen(word_global));
        send(player1, reponse, strlen(reponse)+1, 0);
        send(player2, reponse, strlen(reponse)+1, 0);

        //Initialize the word display with underscores
        char lettresTrouvees[50];
        for(int i=0; i<strlen(word_global); i++){
            lettresTrouvees[i] = '_';
        }
        lettresTrouvees[strlen(word_global)] = '\0';
        int lettresRestantes = strlen(word_global);

        //Game loop
        while(1){
            //Receive guess from player 2
            char messageRecu[LG_MESSAGE];
            memset(messageRecu, 0, LG_MESSAGE);
            int lus = recv(player2, messageRecu, LG_MESSAGE, 0);
            if(lus <= 0){
                printf("Joueur déconnecté.\n");
                break;
            }

            //Array to store indices of letters found or special codes
            int tab[50];
            test_input_game(&game, messageRecu, tab);

            //Incorrect guess -> lose a life
            if(tab[0] == -1){
                game.nb_life--;
                if(game.nb_life <= 0){
                    sprintf(reponse, "lost %s", word_global);
                    send(player1, reponse, strlen(reponse)+1, 0);
                    send(player2, reponse, strlen(reponse)+1, 0);
                    printf("Joueur perdant, plus de vies.\n");
                    break;
                }
                sprintf(reponse, "notfound %d", game.nb_life);
            }
            //Correct guess and word complete -> win
            else if(tab[0] == 100){
                sprintf(reponse, "win");
                send(player1, reponse, strlen(reponse)+1, 0);
                send(player2, reponse, strlen(reponse)+1, 0);
                printf("Joueur gagnant !\n");
                break;
            }
            //Correct letter -> update displayed word
            else {
                int lettres_trouvees_cette_tour = 0;
                for(int i=1; i<=tab[0]; i++){
                    if(lettresTrouvees[tab[i]] == '_'){
                        lettresTrouvees[tab[i]] = word_global[tab[i]];
                        lettres_trouvees_cette_tour++;
                    }
                }
                lettresRestantes -= lettres_trouvees_cette_tour;

                //All letters found -> win
                if(lettresRestantes == 0){
                    sprintf(reponse, "win");
                } else {
                    sprintf(reponse, "%s %d", lettresTrouvees, game.nb_life);
                }
            }

            //Send current state to both players
            send(player1, reponse, strlen(reponse)+1, 0);
            send(player2, reponse, strlen(reponse)+1, 0);
        }

        //Close connections with both players
        close(player1);
        close(player2);
        printf("Fin de la session client.\n");
    }

    //Close listening socket
    close(socketEcoute);
    return 0;
}
