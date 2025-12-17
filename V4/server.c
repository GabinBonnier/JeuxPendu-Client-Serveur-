// server.c — PN V4 prêt pour LAN
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 5003
#define PORT_GAME 6000
#define BUF 256

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY; // écoute toutes les interfaces
    serv.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(sock, 10);
    printf("Serveur PN V4 prêt (port %d)\n", PORT);

    while (1) {
        struct sockaddr_in c1, c2;
        socklen_t l1 = sizeof(c1), l2 = sizeof(c2);

        int s1 = accept(sock, (struct sockaddr*)&c1, &l1);
        printf("Joueur 1 connecté (%s)\n", inet_ntoa(c1.sin_addr)); 

        int s2 = accept(sock, (struct sockaddr*)&c2, &l2);
        printf("Joueur 2 connecté (%s)\n", inet_ntoa(c2.sin_addr));

        char msg[BUF];

        // Envoie aux clients le rôle + IP et port de l’autre joueur
        sprintf(msg, "P1 %s %d", inet_ntoa(c2.sin_addr), PORT_GAME);
        send(s1, msg, strlen(msg)+1, 0);

        sprintf(msg, "P2 %s %d", inet_ntoa(c1.sin_addr), PORT_GAME);
        send(s2, msg, strlen(msg)+1, 0);

        close(s1);
        close(s2);

        printf("Joueurs mis en relation, serveur libre\n");
    }

    close(sock);
    return 0;
}
