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
    printf("Socket créée ! (%d)\n", socketEcoute);

    // === PERMET LA RÉUTILISATION IMMÉDIATE DU PORT ===
    int opt = 1;
    if(setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt");
        exit(-1);
    }

    // Prépare l’adresse locale
    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = PF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    if(bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0){
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès !\n");

    if(listen(socketEcoute, 1) < 0){
        perror("listen");
        exit(-3);
    }
    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Une seule partie → une seule connexion
    socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
    if (socketDialogue < 0) {
        perror("accept");
        exit(-4);
    }

    // Mot choisi
    Game game;
    init_game(&game);
    static char word_global[50];
    strcpy(word_global, game.secret_word);

    printf("Mot choisi : %s\n", word_global);

    // Envoi "start x"
    sprintf(reponse, "start %ld", strlen(word_global));
    send(socketDialogue, reponse, strlen(reponse)+1, 0);

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

        int tab[50];
        test_input_game(&game, messageRecu, tab);

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

        if(strcmp(reponse, "win") == 0 || lettresRestantes == 0){
            printf("Partie terminée, arrêt du serveur.\n");
            break;
        }
    }

    close(socketDialogue);
    close(socketEcoute);
    return 0;
}
