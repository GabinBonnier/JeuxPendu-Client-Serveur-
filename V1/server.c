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

// Securely sending a line ending with \n
void send_line(int sock, const char *msg) {
    char buf[LG_MESSAGE];
    snprintf(buf, LG_MESSAGE, "%s\n", msg);
    send(sock, buf, strlen(buf), 0);
}

int main() {
    int sockEcoute;
    struct sockaddr_in addrLocal;
    socklen_t tailleAddr = sizeof(addrLocal);

    // Socket creation
    sockEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (sockEcoute < 0) { perror("socket"); exit(-1); }

    int opt = 1;
    setsockopt(sockEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addrLocal, 0, tailleAddr);
    addrLocal.sin_family = AF_INET;
    addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    addrLocal.sin_port = htons(PORT);

    if (bind(sockEcoute, (struct sockaddr*)&addrLocal, tailleAddr) < 0) {
        perror("bind");
        exit(-2);
    }

    if (listen(sockEcoute, 2) < 0) {
        perror("listen");
        exit(-3);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {

        int sock1, sock2;
        struct sockaddr_in addrClient1, addrClient2;
        socklen_t tailleClient = sizeof(struct sockaddr_in);

        printf("\nEn attente du joueur 1...\n");
        sock1 = accept(sockEcoute, (struct sockaddr*)&addrClient1, &tailleClient);
        printf("Joueur 1 connecté.\n");

        printf("En attente du joueur 2...\n");
        sock2 = accept(sockEcoute, (struct sockaddr*)&addrClient2, &tailleClient);
        printf("Joueur 2 connecté.\n");

        // -------- START OF THE GAME --------
        char motSecret[] = "mot"; 
        int taille = strlen(motSecret);

        Game game1, game2;
        init_game(&game1, motSecret);
        init_game(&game2, motSecret);

        char lettresTrouvees[50];
        for (int i = 0; i < taille; i++) lettresTrouvees[i] = '_';
        lettresTrouvees[taille] = '\0';

        char startmsg[50];
        snprintf(startmsg, 50, "start %d", taille);

        send_line(sock1, startmsg);
        send_line(sock2, startmsg);

        int tour = 1;
        int finJeu = 0;

        // -------- PARTY LOOP --------
        while (!finJeu) {

            int sockActuel = (tour == 1 ? sock1 : sock2);
            int sockAutre  = (tour == 1 ? sock2 : sock1);
            Game *gameAct  = (tour == 1 ? &game1 : &game2);

            // Say who's playing
            send_line(sockActuel, "YOUR_TURN");
            send_line(sockAutre, "WAIT");

            // Read player entry
            char buffer[LG_MESSAGE];
            int lus = recv(sockActuel, buffer, LG_MESSAGE - 1, 0);
            if (lus <= 0) {
                printf("Client %d déconnecté.\n", tour);
                break;
            }
            buffer[lus] = '\0';
            buffer[strcspn(buffer, "\r\n")] = '\0';

            int tab[50];
            test_input_game(gameAct, buffer, tab);

            char msgActuel[LG_MESSAGE];
            char msgAutre[LG_MESSAGE];

            // ---------- INCORRECT LETTER ----------
            if (tab[0] == -1) {

                gameAct->nb_life--;
                snprintf(msgActuel, LG_MESSAGE, "notfound %d", gameAct->nb_life);
                snprintf(msgAutre, LG_MESSAGE,
                         "UPDATE L'autre joueur n'a rien trouvé. Mot : %s",
                         lettresTrouvees);

                if (gameAct->nb_life <= 0) {
                    snprintf(msgActuel, LG_MESSAGE, "lost %s", motSecret);
                    send_line(sockActuel, msgActuel);
                    send_line(sockAutre, "END L'autre joueur a perdu !");
                    finJeu = 1;
                    break;
                }
            }

            // ---------- FULL WORD TYPE ----------
            else if (tab[0] == 100) {
                snprintf(msgActuel, LG_MESSAGE, "win %s", motSecret);
                send_line(sockActuel, msgActuel);
                send_line(sockAutre, "END L'autre joueur a gagné !");
                finJeu = 1;
                break;
            }

            // ---------- LETTER(S) FOUND ----------
            else {

                // Update of letters found
                for (int i = 1; i <= tab[0]; i++)
                    lettresTrouvees[tab[i]] = motSecret[tab[i]];

                // NEW CONDITION: word fully revealed
                if (strcmp(lettresTrouvees, motSecret) == 0) {
                    snprintf(msgActuel, LG_MESSAGE, "win %s", motSecret);
                    send_line(sockActuel, msgActuel);
                    send_line(sockAutre, "END L'autre joueur a gagné !");
                    finJeu = 1;
                    break;
                }

                // Normal update
                snprintf(msgActuel, LG_MESSAGE, "%s %d",
                         lettresTrouvees, gameAct->nb_life);

                snprintf(msgAutre, LG_MESSAGE,
                         "UPDATE L'autre joueur a trouvé : %s",
                         lettresTrouvees);
            }

            // Sending prepared messages
            send_line(sockActuel, msgActuel);
            send_line(sockAutre, msgAutre);

            // Change of tower
            tour = (tour == 1 ? 2 : 1);
        }

        close(sock1);
        close(sock2);
        printf("Partie terminée.\n");
    }

    close(sockEcoute);
    return 0;
}
