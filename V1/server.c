#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "game.h"

#define PORT 5003
#define LG_MESSAGE 256

int main() {
    int sockEcoute;
    struct sockaddr_in addrLocal;
    socklen_t tailleAddr = sizeof(addrLocal);

    // Création socket
    sockEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (sockEcoute < 0) { perror("socket"); exit(-1); }

    int opt = 1;
    setsockopt(sockEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addrLocal, 0, tailleAddr);
    addrLocal.sin_family = AF_INET;
    addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    addrLocal.sin_port = htons(PORT);

    if (bind(sockEcoute, (struct sockaddr*)&addrLocal, tailleAddr) < 0) {
        perror("bind"); exit(-2);
    }

    if (listen(sockEcoute, 2) < 0) {
        perror("listen"); exit(-3);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        int sock1, sock2;
        struct sockaddr_in addrClient1, addrClient2;
        socklen_t tailleClient = sizeof(struct sockaddr_in);

        printf("\nEn attente du client 1...\n");
        sock1 = accept(sockEcoute, (struct sockaddr*)&addrClient1, &tailleClient);
        printf("Client 1 connecté : %s:%d\n", inet_ntoa(addrClient1.sin_addr), ntohs(addrClient1.sin_port));

        printf("En attente du client 2...\n");
        sock2 = accept(sockEcoute, (struct sockaddr*)&addrClient2, &tailleClient);
        printf("Client 2 connecté : %s:%d\n", inet_ntoa(addrClient2.sin_addr), ntohs(addrClient2.sin_port));

        // --- Initialisation du jeu ---
        Game game1, game2;
        char motSecret[] = "mot"; // Mot fixe pour la partie
        init_game(&game1, motSecret);
        init_game(&game2, motSecret);

        printf("Les deux clients sont connectés. Démarrage de la partie...\n");
        printf("---------------------------------------------------\n");
        printf("La partie vient de commencer et le mot secret choisi est %s\n", motSecret);
        printf("Le mot comporte %ld lettres.\n", strlen(motSecret));

        char motGlobal[50];
        strcpy(motGlobal, game1.secret_word);

        char lettresTrouvees[50];
        for (int i=0; i<strlen(motGlobal); i++) lettresTrouvees[i] = '_';
        lettresTrouvees[strlen(motGlobal)] = '\0';

        int tour = 2; // Joueur 1 commence
        int finJeu = 0;

        // Envoi message "start x" à tous les clients
        char msg[LG_MESSAGE];
        snprintf(msg, LG_MESSAGE, "start %ld", strlen(motGlobal));
        send(sock1, msg, strlen(msg)+1, 0);
        send(sock2, msg, strlen(msg)+1, 0);

        while (!finJeu) {
            int sockActuel = (tour == 1) ? sock1 : sock2;
            int sockAutre = (tour == 1) ? sock2 : sock1;
            Game *gameActuel = (tour == 1) ? &game1 : &game2;

            // Informer les joueurs sur leur tour
            send(sockActuel, "YOUR_TURN", strlen("YOUR_TURN")+1, 0);
            send(sockAutre, "WAIT", strlen("WAIT")+1, 0);

            // --- LOG côté serveur ---
            printf("\nTour du joueur %d\n", tour);
            printf("Joueur %d : C'est votre tour !\n", tour);
            printf("Joueur %d : Ce n'est pas votre tour ! Patientez...\n", (tour == 1) ? 2 : 1);

            // Recevoir la lettre/mot du joueur
            char buffer[LG_MESSAGE];
            memset(buffer, 0, LG_MESSAGE);
            int lus = recv(sockActuel, buffer, LG_MESSAGE, 0);
            if (lus <= 0) {
                printf("Client %d déconnecté.\n", tour);
                break;
            }
            buffer[strcspn(buffer, "\n")] = '\0';

            // Tester la lettre/mot
            int tab[50];
            test_input_game(gameActuel, buffer, tab);

            if (tab[0] == -1) {
                gameActuel->nb_life--;
                if (gameActuel->nb_life <= 0) {
                    snprintf(msg, LG_MESSAGE, "lost %s", motGlobal);
                    send(sockActuel, msg, strlen(msg)+1, 0);
                    send(sockAutre, "END L'autre joueur a perdu !", strlen("END L'autre joueur a perdu !")+1, 0);
                    finJeu = 1;
                    break;
                }
                snprintf(msg, LG_MESSAGE, "notfound %d", gameActuel->nb_life);
            } else if (tab[0] == 100) {
                snprintf(msg, LG_MESSAGE, "win %s", motGlobal);
                send(sockActuel, msg, strlen(msg)+1, 0);
                send(sockAutre, "END L'autre joueur a gagné !", strlen("END L'autre joueur a gagné !")+1, 0);
                finJeu = 1;
                break;
            } else {
                for (int i=1; i<=tab[0]; i++) lettresTrouvees[tab[i]] = motGlobal[tab[i]];
                snprintf(msg, LG_MESSAGE, "%s %d", lettresTrouvees, gameActuel->nb_life);
            }

            send(sockActuel, msg, strlen(msg)+1, 0);

            // Changer de tour
            tour = (tour == 1) ? 2 : 1;
        }

        close(sock1);
        close(sock2);
        printf("\nPartie terminée, en attente de nouveaux joueurs...\n");
    }

    close(sockEcoute);
    return 0;
}
