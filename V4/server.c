// server.c — PN V4 STRICT
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
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(PORT);

    bind(sock, (struct sockaddr*)&serv, sizeof(serv));
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

        // Vers joueur 1 : rôle + IP joueur 2
        sprintf(msg, "P1 %s %d", inet_ntoa(c2.sin_addr), PORT_GAME);
        send(s1, msg, strlen(msg) + 1, 0);

        // Vers joueur 2 : rôle + IP joueur 1
        sprintf(msg, "P2 %s %d", inet_ntoa(c1.sin_addr), PORT_GAME);
        send(s2, msg, strlen(msg) + 1, 0);

        close(s1);
        close(s2);

        printf("Joueurs mis en relation, serveur libre\n");
    }
}
