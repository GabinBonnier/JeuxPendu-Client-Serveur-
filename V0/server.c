#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5002
#define LG_MESSAGE 256

#include "game.c"
#include "protocole.h"

int main(){
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;

    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;

    char messageRecu[LG_MESSAGE];
    char reponse[LG_MESSAGE];
    int lus;

    // Création socket
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if(socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    if(bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0){
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès !\n");

    if(listen(socketEcoute, 5) < 0){
        perror("listen");
        exit(-3);
    }
    printf("Socket placée en écoute passive ...\n");

    while(1){
        printf("Attente d’une demande de connexion (quitter avec Ctrl-C)\n");

        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue < 0) {
            perror("accept");
            continue;
        }

        // Mot choisi
        char *word_init = def_word_secret();
        static char word_global[50];
        strcpy(word_global, word_init);

        printf("Mot choisi : %s\n", word_global);

        // Envoi "start x"
        sprintf(reponse, "start %ld", strlen(word_global));
        send(socketDialogue, reponse, strlen(reponse)+1, 0);
        printf("Envoyé au client : %s\n", reponse);

        // Tableau pour suivre lettres trouvées
        char lettresTrouvees[50];
        for(int i=0; i<strlen(word_global); i++) lettresTrouvees[i] = '_';
        lettresTrouvees[strlen(word_global)] = '\0';

        int lettresRestantes = strlen(word_global);

        // ========= BOUCLE DE JEU ==========
        while(1){
            memset(messageRecu, 0, LG_MESSAGE);
            lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0);
            if(lus <= 0){
                printf("Client déconnecté.\n");
                break;
            }

            printf("Message reçu : %s (%d octets)\n", messageRecu, lus);

            int tab[50];
            test_input(word_global, messageRecu, tab);

            // Mise à jour lettres trouvées
            if(tab[0] == -1){
                sprintf(reponse, "notfound");
            } else if(tab[0] == 100){
                sprintf(reponse, "win");
            } else {
                for(int i=1; i<=tab[0]; i++){
                    lettresTrouvees[tab[i]] = word_global[tab[i]];
                }
                lettresRestantes -= tab[0];
                sprintf(reponse, "%s", lettresTrouvees);
            }

            send(socketDialogue, reponse, strlen(reponse)+1, 0);
            printf("Réponse envoyée : %s\n", reponse);

            if(strcmp(reponse, "win") == 0 || lettresRestantes == 0){
                break;
            }
        }

        close(socketDialogue);
    }

    close(socketEcoute);
    return 0;
}
