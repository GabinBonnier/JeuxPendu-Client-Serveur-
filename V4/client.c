// client.c — PN V4 (Player 1 / Player 2)
// Fonctionne avec server.c et game.c

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

    /* ================= CONNEXION AU SERVEUR ================= */
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

    recv(sockServ, buffer, BUF, 0);
    close(sockServ);

    sscanf(buffer, "%s %s %d", role, ipPeer, &portPeer);
    int isPlayer1 = (strcmp(role, "P1") == 0);

    int sockGame;

    /* ================= PLAYER 1 ================= */
    if (isPlayer1) {

        sockGame = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(portPeer);

        if (bind(sockGame, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            return 1;
        }

        listen(sockGame, 1);
        printf("Player 1 : en attente du joueur 2...\n");

        sockGame = accept(sockGame, NULL, NULL);

        char mot[BUF];
        printf("Choisissez le mot secret : ");
        fgets(mot, BUF, stdin);
        mot[strcspn(mot, "\n")] = 0;

        send(sockGame, mot, strlen(mot) + 1, 0);

        while (1) {
            char prop[BUF];
            int n = recv(sockGame, prop, BUF, 0);
            if (n <= 0) break;

            if (!strcmp(prop, "GAGNE")) {
                printf("🎉 Le joueur 2 a gagné !\n");
                break;
            }
            if (!strcmp(prop, "PERDU")) {
                printf("❌ Le joueur 2 a perdu.\n");
                break;
            }

            printf("Joueur 2 propose : %s\n", prop);
        }
    }

    /* ================= PLAYER 2 ================= */
    else {
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
            printf("\nMot : %s | Vies : %d\n", affiche, vies);
            printf("Lettre : ");

            char c;
            scanf(" %c", &c);

            char prop[2] = {c, '\0'};
            send(sockGame, prop, 2, 0);

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
                printf("🎉 Gagné !\n");
                break;
            }
        }

        if (vies == 0) {
            send(sockGame, "PERDU", 6, 0);
            printf("❌ Perdu ! Mot : %s\n", mot);
        }
    }

    close(sockGame);
    return 0;
}
