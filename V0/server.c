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
    socklen_t longueurAdresse = sizeof(pointDeRencontreLocal);

    // Création de la socket d'écoute
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if(socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée ! (%d)\n", socketEcoute);

    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = AF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    if(bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0){
        perror("bind");
        exit(-2);
    }

    if(listen(socketEcoute, 5) < 0){
        perror("listen");
        exit(-3);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Boucle infinie pour accepter plusieurs clients
    while(1) {
        int socketDialogue;
        struct sockaddr_in pointDeRencontreDistant;
        socklen_t tailleDistant = sizeof(pointDeRencontreDistant);

        printf("\nEn attente d'un nouveau client...\n");
        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &tailleDistant);
        if(socketDialogue < 0){
            perror("accept");
            continue; // Ne pas quitter le serveur
        }

        printf("Client connecté : %s:%d\n",
               inet_ntoa(pointDeRencontreDistant.sin_addr),
               ntohs(pointDeRencontreDistant.sin_port));

        // Initialisation du jeu
        Game game;
        init_game(&game);
        char word_global[50];
		printf("Le mot comporte %d lettres. \n", (int)strlen(game.secret_word));
        strcpy(word_global, game.secret_word);
        printf("Mot choisi : %s\n", word_global);

        char reponse[LG_MESSAGE];
        sprintf(reponse, "start %ld", strlen(word_global));
        send(socketDialogue, reponse, strlen(reponse)+1, 0);

        char lettresTrouvees[50];
        for(int i=0; i<strlen(word_global); i++) lettresTrouvees[i] = '_';
        lettresTrouvees[strlen(word_global)] = '\0';

        int lettresRestantes = strlen(word_global);
        char messageRecu[LG_MESSAGE];

        // Boucle de jeu pour ce client
        while(1){
            memset(messageRecu, 0, LG_MESSAGE);
            int lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0);

            if(lus <= 0){
                printf("Client déconnecté.\n");
                break;
            }

            int tab[50];
            test_input_game(&game, messageRecu, tab);

            if(tab[0] == -1){
                game.nb_life--;
                if(game.nb_life <= 0){
                    sprintf(reponse, "lost %s", word_global);
                    send(socketDialogue, reponse, strlen(reponse)+1, 0);
                    printf("Joueur perdant, plus de vies.\n");
                    break;
                }
                sprintf(reponse, "notfound %d", game.nb_life);
            }
            else if(tab[0] == 100){
                sprintf(reponse, "win");
            }
            else {
                for(int i=1; i<=tab[0]; i++){
                    lettresTrouvees[tab[i]] = word_global[tab[i]];
                }
                lettresRestantes -= tab[0];
                sprintf(reponse, "%s %d", lettresTrouvees, game.nb_life);
            }

            send(socketDialogue, reponse, strlen(reponse)+1, 0);

            if(strcmp(reponse, "win") == 0){
                printf("Joueur gagnant !\n");
                break;
            }

            if(lettresRestantes == 0){
                printf("Mot trouvé par le joueur.\n");
                break;
            }
        }

        close(socketDialogue);
        printf("Fin de la session client.\n");
    }

    close(socketEcoute);
    return 0;
}
