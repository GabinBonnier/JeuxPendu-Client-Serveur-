#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "game.h"

#define LG_MESSAGE 256

// Lecture d'une ligne terminée par \n
int recv_line(int sock, char *buffer, int max) {
    int i = 0;
    char c;

    while (i < max - 1) {
        int n = recv(sock, &c, 1, 0);
        if (n <= 0) return n;
        if (c == '\n') break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    return i;
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("USAGE : %s ip port\n", argv[0]);
        return 0;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &addr.sin_addr);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    printf("Connecté au serveur.\n");

    char buffer[LG_MESSAGE];

    // --- Réception du message start ---
    recv_line(sock, buffer, LG_MESSAGE);

    int tailleMot;
    sscanf(buffer, "start %d", &tailleMot);

    char motAffiche[50];
    for (int i=0; i<tailleMot; i++) motAffiche[i] = '_';
    motAffiche[tailleMot] = '\0';

    printf("Mot à deviner (%d lettres) : %s\n", tailleMot, motAffiche);

    // ----- BOUCLE PRINCIPALE -----
    while (1) {

        if (recv_line(sock, buffer, LG_MESSAGE) <= 0) break;

        // ----- UPDATE -----
        if (strncmp(buffer, "UPDATE", 6) == 0) {
            printf("%s\n", buffer + 7);
            continue;
        }

        // ----- WAIT -----
        if (strcmp(buffer, "WAIT") == 0) {
            printf("En attente de l’autre joueur...\n");
            continue;
        }

        // ----- YOUR TURN -----
        if (strcmp(buffer, "YOUR_TURN") == 0) {
            printf("C'est votre tour : ");
            fgets(buffer, LG_MESSAGE, stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            send(sock, buffer, strlen(buffer), 0);
            continue;
        }

        // ----- NOTFOUND -----
        if (strncmp(buffer, "notfound", 8) == 0) {
            int vies;
            sscanf(buffer, "notfound %d", &vies);
            printf("Lettre incorrecte. Vies restantes : %d\n", vies);
            affichage(vies);
            continue;
        }

        // ----- LOST -----
        if (strncmp(buffer, "lost", 4) == 0) {
            char mot[50];
            sscanf(buffer, "lost %s", mot);
            printf("Vous avez perdu ! Le mot était : %s\n", mot);
            break;
        }

        // ----- WIN -----
        if (strncmp(buffer, "win", 3) == 0) {
            char mot[50];
            sscanf(buffer, "win %s", mot);
            printf("Bravo ! Vous avez trouvé : %s\n", mot);
            break;
        }

        // ----- END -----
        if (strncmp(buffer, "END", 3) == 0) {
            printf("%s\n", buffer + 4);
            break;
        }

        // ----- MISE À JOUR DU MOT -----
        char temp[50];
        int vies;
        if (sscanf(buffer, "%s %d", temp, &vies) == 2) {
            strcpy(motAffiche, temp);
            printf("Mot : %s | Vies : %d\n", motAffiche, vies);
            affichage(vies);
        }
    }

    close(sock);
    return 0;
}
