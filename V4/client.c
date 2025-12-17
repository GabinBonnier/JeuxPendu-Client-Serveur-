#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "game.h"

#define BUF 256
#define VIES 6

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s ipServeur portServeur\n", argv[0]);
        return 1;
    }

    // connexion serveur
    int sockServ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockServ < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &serv.sin_addr);

    if (connect(sockServ, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("connect serveur");
        return 1;
    }

    char buffer[BUF], role[3], ipPeer[16];
    int portPeer;

    printf("Joueur 1 connecté : en attente du joueur 2...\n"); 

    recv(sockServ, buffer, BUF, 0);
    close(sockServ);

    sscanf(buffer, "%s %s %d", role, ipPeer, &portPeer);
    int isPlayer1 = (strcmp(role, "P1") == 0);

    int sockGame;

    // Player 1
    if (isPlayer1) {
          printf("Les deux joueurs sont connectés ! \n");
          printf("Début de la partie ! \n\n");
        
        
        sockGame = socket(AF_INET, SOCK_STREAM, 0);

        int opt = 1;
        setsockopt(sockGame, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(portPeer);
        
        
        if (bind(sockGame, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            return 1;
        }
        
        listen(sockGame, 1);
        sockGame = accept(sockGame, NULL, NULL);

        char mot[BUF];
        printf("Choisissez le mot secret : ");
        fgets(mot, BUF, stdin);
        mot[strcspn(mot, "\n")] = 0;

        send(sockGame, mot, strlen(mot) + 1, 0);
        printf("Le mot comporte %d lettres\n", (int)strlen(mot));

        while (1) {
            char prop[BUF];
            int n = recv(sockGame, prop, BUF - 1, 0);
            if (n <= 0) break;

            prop[n] = '\0';

            if (!strcmp(prop, "GAGNE")) {
                printf(
                    "Malheureusement vous avez perdu... "
                    "Pour la prochaine partie choisissez un mot plus difficile que : %s !\n",
                    mot
                );
                send(sockGame, "STOP", 5, 0);
                break;
            }

            if (!strcmp(prop, "PERDU")) {
                printf("Vous avez gagné, le joueur 2 n'a pas trouvé votre mot.\n");
                send(sockGame, "STOP", 5, 0);
                break;
            }

            printf("Joueur 2 propose : %s\n", prop);
        }
    }

    // player 2
    else {
        printf("Vous êtes connectés avec succès ! \n");
        printf("Les deux joueurs sont connectés : la partie peut commencer ! \n\n");

        sleep(1);

        sockGame = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(portPeer);
        inet_aton(ipPeer, &addr.sin_addr);

        if (connect(sockGame, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("connect P1");
            return 1;
        }

        printf("En attente du choix du mot...\n");

        char mot[BUF];
        recv(sockGame, mot, BUF, 0);

        int len = strlen(mot);
        char affiche[BUF];
        memset(affiche, '_', len);
        affiche[len] = '\0';

        int vies = VIES;

        while (vies > 0) {
            printf("\nMot : ");
            for (int i = 0; i < len; i++) {
                printf("%c ", affiche[i]); 
            }
            printf("| Vies : %d | Le mot comporte %d lettres \n", vies, len);

            printf("\n Proposition (lettre ou mot) : ");

            char prop[BUF];
            scanf("%s", prop);
            send(sockGame, prop, strlen(prop) + 1, 0);

            if (!strcmp(prop, mot)) {
                send(sockGame, "GAGNE", 6, 0);
                printf("Félicitations, vous avez gagné la partie !\n");

                char fin[BUF];
                recv(sockGame, fin, BUF, 0); 
                break;
            }

            if (strlen(prop) == 1) {
                char c = prop[0];
                int trouve = 0;

                for (int i = 0; i < len; i++) {
                    if (mot[i] == c) {
                        affiche[i] = c;
                        trouve = 1;
                    }
                }

                if (!trouve) {
                    vies--;
                    affichage(vies);
                }

                if (!strcmp(mot, affiche)) {
                    send(sockGame, "GAGNE", 6, 0);
                    printf("Félicitations, vous avez gagné la partie !\n");

                    char fin[BUF];
                    recv(sockGame, fin, BUF, 0);
                    break;
                }
            }
        }

        if (vies == 0) {
            send(sockGame, "PERDU", 6, 0);
            printf("Vous avez perdu, le mot était : %s\n", mot);

            char fin[BUF];
            recv(sockGame, fin, BUF, 0);
        }
    }

    shutdown(sockGame, SHUT_RDWR);
    close(sockGame);
    return 0;
}
