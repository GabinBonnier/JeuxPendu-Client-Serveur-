#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5003
#define LG_MESSAGE 512

int main() {
    int sockListen = socket(AF_INET, SOCK_STREAM, 0);
    if(sockListen < 0){ perror("socket"); exit(-1); }

    int opt = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addrLocal;
    memset(&addrLocal, 0, sizeof(addrLocal));
    addrLocal.sin_family = AF_INET;
    addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    addrLocal.sin_port = htons(PORT);

    if(bind(sockListen, (struct sockaddr *)&addrLocal, sizeof(addrLocal)) < 0){ perror("bind"); exit(-2); }
    if(listen(sockListen, 5) < 0){ perror("listen"); exit(-3); }

    printf("Serveur PN V4 en écoute sur le port %d...\n", PORT);

    while(1) {
        struct sockaddr_in addr1, addr2;
        socklen_t l1 = sizeof(addr1), l2 = sizeof(addr2);

        printf("En attente de joueurs...\n");
        int player1 = accept(sockListen, (struct sockaddr *)&addr1, &l1);
        if(player1 < 0){ perror("accept joueur1"); continue; }
        printf("Joueur 1 connecté\n");

        int player2 = accept(sockListen, (struct sockaddr *)&addr2, &l2);
        if(player2 < 0){ perror("accept joueur2"); close(player1); continue; }
        printf("Joueur 2 connecté\n");

        // Attribution des rôles
        send(player1, "choose", 7, 0); // joueur 1 choisit le mot
        send(player2, "start", 6, 0);  // joueur 2 devine

        char motSecret[256];
        int nb = recv(player1, motSecret, sizeof(motSecret), 0);
        if(nb <= 0){ close(player1); close(player2); continue; }
        motSecret[nb] = '\0';

        int vies = 6;
        int tailleMot = strlen(motSecret);
        char motAffiche[256];
        for(int i=0;i<tailleMot;i++) motAffiche[i] = '_';
        motAffiche[tailleMot]='\0';

        send(player2, motSecret, strlen(motSecret)+1, 0);

        while(1){
            char buffer[LG_MESSAGE];
            nb = recv(player2, buffer, sizeof(buffer), 0);
            if(nb <= 0) break;
            buffer[nb]='\0';

            if(strlen(buffer)==1){ // Lettre
                char c = buffer[0];
                int trouve = 0;
                for(int i=0;i<tailleMot;i++){
                    if(motSecret[i]==c && motAffiche[i]=='_'){
                        motAffiche[i]=c;
                        trouve++;
                    }
                }

                // Vérifier si toutes les lettres sont trouvées (victoire automatique)
                int gagne = 1;
                for(int i=0;i<tailleMot;i++){
                    if(motAffiche[i]=='_'){
                        gagne = 0;
                        break;
                    }
                }
                if(gagne){
                    snprintf(buffer, LG_MESSAGE, "win");
                    send(player1, buffer, strlen(buffer)+1, 0);
                    send(player2, buffer, strlen(buffer)+1, 0);
                    break;
                }

                if(trouve>0){
                    snprintf(buffer, LG_MESSAGE, "%s %d", motAffiche, vies);
                    send(player1, buffer, strlen(buffer)+1, 0);
                    send(player2, buffer, strlen(buffer)+1, 0);
                } else {
                    vies--;
                    snprintf(buffer, LG_MESSAGE, "notfound %d", vies);
                    send(player1, buffer, strlen(buffer)+1, 0);
                    send(player2, buffer, strlen(buffer)+1, 0);
                    if(vies <= 0) {
                        snprintf(buffer, LG_MESSAGE, "lost %s", motSecret);
                        send(player1, buffer, strlen(buffer)+1, 0);
                        send(player2, buffer, strlen(buffer)+1, 0);
                        break;
                    }
                }
            } else { // Mot complet
                if(strcmp(buffer, motSecret)==0){
                    snprintf(buffer, LG_MESSAGE, "win");
                    send(player1, buffer, strlen(buffer)+1, 0);
                    send(player2, buffer, strlen(buffer)+1, 0);
                    break;
                } else {
                    vies--;
                    if(vies <=0){
                        snprintf(buffer, LG_MESSAGE, "lost %s", motSecret);
                        send(player1, buffer, strlen(buffer)+1, 0);
                        send(player2, buffer, strlen(buffer)+1, 0);
                        break;
                    } else {
                        snprintf(buffer, LG_MESSAGE, "notfound %d", vies);
                        send(player1, buffer, strlen(buffer)+1, 0);
                        send(player2, buffer, strlen(buffer)+1, 0);
                    }
                }
            }
        }

        close(player1);
        close(player2);
        printf("Fin de la partie.\n");
    }

    close(sockListen);
    return 0;
}
