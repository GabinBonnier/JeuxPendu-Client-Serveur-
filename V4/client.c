#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "game.h"  


#define LG_MESSAGE 256


int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("USAGE : %s ip port\n", argv[0]);
        exit(-1);
    }

    char ipDest[16];
    int portDest;
    strncpy(ipDest, argv[1], 16);
    sscanf(argv[2], "%d", &portDest);

    int sock;
    struct sockaddr_in serverAddr;
    socklen_t len = sizeof(serverAddr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){ perror("socket"); exit(-1); }

    memset(&serverAddr, 0, len);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portDest);
    inet_aton(ipDest, &serverAddr.sin_addr);

    if(connect(sock, (struct sockaddr*)&serverAddr, len) < 0){
        perror("connect");
        exit(-2);
    }

    printf("Connexion au serveur %s:%d réussie !\n", ipDest, portDest);

    char buffer[LG_MESSAGE];
    int nb;

    // Recevoir le rôle
    nb = recv(sock, buffer, LG_MESSAGE, 0);
    if(nb <= 0){ printf("Connexion interrompue.\n"); close(sock); return 0; }
    buffer[nb] = '\0';

    int role;
    if(strcmp(buffer, "choose") == 0){
        role = 1;
        printf("Vous êtes le joueur 1 (choix du mot)\n");
    } else if(strcmp(buffer, "start") == 0){
        role = 2;
        printf("Vous êtes le joueur 2 (deviner le mot)\n");
    } else {
        printf("Message inconnu du serveur : %s\n", buffer);
        close(sock);
        return 0;
    }

    char motSecret[256];
    char motAffiche[256];
    int vies = 6;

    if(role == 1) {
        // Joueur 1 choisit le mot
        printf("Entrez le mot secret : ");
        fgets(motSecret, sizeof(motSecret), stdin);
        motSecret[strcspn(motSecret, "\n")] = 0;

        send(sock, motSecret, strlen(motSecret)+1, 0);

        int tailleMot = strlen(motSecret);
        for(int i=0;i<tailleMot;i++) motAffiche[i]='_';
        motAffiche[tailleMot]='\0';

        printf("Mot choisi : %s (%d lettres)\n", motAffiche, tailleMot);

        while(1){
            nb = recv(sock, buffer, LG_MESSAGE, 0);
            if(nb <= 0){ printf("Connexion interrompue.\n"); break; }
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
                if(trouve>0) send(sock, motAffiche, strlen(motAffiche)+1, 0);
                else {
                    vies--;
                    printf("Lettre incorrecte ! ");
                    affichage(vies);
                    send(sock, "incorrect", 10, 0);
                }
            } else { // Mot complet
                if(strcmp(buffer, motSecret)==0){
                    printf("Le joueur 2 a trouvé le mot !\n");
                    send(sock, "win", 4, 0);
                    break;
                } else {
                    vies--;
                    printf("Mot incorrect ! ");
                    affichage(vies);
                    send(sock, "incorrect", 10, 0);
                }
            }

            if(vies<=0){
                printf("Le joueur 2 n'a plus de vies ! Le mot était : %s\n", motSecret);
                send(sock, "lost", 5, 0);
                break;
            }
        }
    } else {
        // Joueur 2
        nb = recv(sock, motSecret, sizeof(motSecret), 0);
        if(nb <= 0){ printf("Connexion interrompue.\n"); close(sock); return 0; }
        motSecret[nb]='\0';

        int tailleMot = strlen(motSecret);
        for(int i=0;i<tailleMot;i++) motAffiche[i]='_';
        motAffiche[tailleMot]='\0';

        printf("Le mot comporte %d lettres : %s\n", tailleMot, motAffiche);

        while(1){
            printf("\nEntrez une lettre ou le mot complet : ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;

            send(sock, buffer, strlen(buffer)+1, 0);

            nb = recv(sock, motAffiche, sizeof(motAffiche), 0);
            if(nb <=0){ printf("Connexion interrompue.\n"); break; }
            motAffiche[nb]='\0';

            if(strcmp(motAffiche,"win")==0){
                printf("Félicitations ! Vous avez trouvé le mot !\n");
                break;
            } else if(strcmp(motAffiche,"lost")==0){
                printf("Plus de vies ! Partie terminée.\n");
                break;
            } else if(strcmp(motAffiche,"incorrect")==0){
                vies--;
                printf("Proposition incorrecte ! ");
                affichage(vies);
                if(vies<=0){
                    printf("Plus de vies ! Partie terminée.\n");
                    break;
                }
            } else {
                printf("Mot actuel : %s\n", motAffiche);
            }
        }
    }

    close(sock);
    return 0;
}
