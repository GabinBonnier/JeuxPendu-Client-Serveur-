#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "game.h"

#define PORT 5003

int main() {
    int sockListen = socket(AF_INET, SOCK_STREAM, 0);
    if(sockListen < 0){ perror("socket"); exit(-1); }

    int opt = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(sockListen, (struct sockaddr*)&addr, sizeof(addr)) < 0){ perror("bind"); exit(-1); }
    if(listen(sockListen, 5) < 0){ perror("listen"); exit(-1); }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while(1){
        struct sockaddr_in j1, j2;
        socklen_t l1 = sizeof(j1), l2 = sizeof(j2);

        int s1 = accept(sockListen, (struct sockaddr*)&j1, &l1);
        printf("Joueur 1 connecté\n");
        int s2 = accept(sockListen, (struct sockaddr*)&j2, &l2);
        printf("Joueur 2 connecté\n");

        Game game;
        char mot[MAX_WORD];
        send(s1, "choose", 7, 0);
        recv(s1, mot, sizeof(mot), 0);
        mot[strcspn(mot, "\n")] = 0;
        init_game(&game, mot);

        char affiche[MAX_WORD];
        for(int i=0; i<game.nb_letters; i++) affiche[i] = '_';
        affiche[game.nb_letters] = '\0';

        char msg[MAX_BUFFER];
        sprintf(msg, "start %d", game.nb_letters);
        send(s1, msg, strlen(msg)+1, 0);
        send(s2, msg, strlen(msg)+1, 0);

        int lettresRestantes = game.nb_letters;

        while(1){
            char buffer[MAX_BUFFER];
            int lus = recv(s2, buffer, sizeof(buffer), 0);
            if(lus <= 0) break;

            int tab[50];
            test_input_game(&game, buffer, tab);

            if(tab[0] == -1){
                game.nb_life--;
                if(game.nb_life <= 0){
                    sprintf(msg, "lost %s", game.secret_word);
                    send(s1, msg, strlen(msg)+1,0);
                    send(s2, msg, strlen(msg)+1,0);
                    break;
                }
                sprintf(msg, "notfound %d", game.nb_life);
            }
            else if(tab[0] == 100){
                sprintf(msg, "win");
                send(s1, msg, strlen(msg)+1,0);
                send(s2, msg, strlen(msg)+1,0);
                break;
            }
            else {
                for(int i=1;i<=tab[0];i++){
                    if(affiche[tab[i]]=='_'){ affiche[tab[i]]=game.secret_word[tab[i]]; lettresRestantes--; }
                }
                if(lettresRestantes==0) sprintf(msg,"win");
                else sprintf(msg,"%s %d", affiche, game.nb_life);
            }
            send(s1, msg, strlen(msg)+1,0);
            send(s2, msg, strlen(msg)+1,0);
        }

        close(s1); close(s2);
        printf("Partie terminée.\n");
    }

    close(sockListen);
    return 0;
}
