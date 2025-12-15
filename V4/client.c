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
    int nb = recv(sock, buffer, LG_MESSAGE, 0);
    if(nb <= 0){ printf("Connexion interrompue.\n"); close(sock); return 0; }
    buffer[nb]='\0';

    int role = (strcmp(buffer,"choose")==0) ? 1 : 2;
    char motSecret[256], motAffiche[256];
    int vies = 6;

    if(role==1){
        // Joueur 1
        printf("Vous êtes le joueur 1. Entrez le mot secret : ");
        fgets(motSecret, sizeof(motSecret), stdin);
        motSecret[strcspn(motSecret,"\n")]=0;
        send(sock, motSecret, strlen(motSecret)+1, 0);

        while(1){
            nb = recv(sock, buffer, LG_MESSAGE, 0);
            if(nb<=0) break;
            buffer[nb]='\0';

            if(strncmp(buffer,"win",3)==0){
                printf("Le joueur 2 a gagné la partie !\n");
                break;
            } else if(strncmp(buffer,"lost",4)==0){
                printf("Plus de vies ! Le mot était : %s\n", buffer+5);
                break;
            } else if(strncmp(buffer,"notfound",8)==0){
                int v;
                sscanf(buffer,"notfound %d",&v);
                vies=v;
                printf("Lettre absente ! ");
                affichage(vies);
            } else {
                int v;
                char tmp[256];
                sscanf(buffer,"%s %d", tmp,&v);
                strcpy(motAffiche,tmp);
                vies=v;
                printf("Mot actuel : %s | ", motAffiche);
                affichage(vies);
            }
        }
    } else {
        // Joueur 2
        nb = recv(sock, motSecret, sizeof(motSecret),0);
        if(nb<=0){ printf("Connexion interrompue.\n"); close(sock); return 0; }
        motSecret[nb]='\0';

        int tailleMot = strlen(motSecret);
        for(int i=0;i<tailleMot;i++) motAffiche[i]='_';
        motAffiche[tailleMot]='\0';

        printf("Vous êtes le joueur 2. Le mot comporte %d lettres.\n", tailleMot);
        printf("Mot actuel : %s | ", motAffiche);
        affichage(vies);

        while(1){
            printf("\nEntrez une lettre ou le mot complet : ");
            fgets(buffer, LG_MESSAGE, stdin);
            buffer[strcspn(buffer,"\n")]=0;
            send(sock, buffer, strlen(buffer)+1,0);

            nb = recv(sock, buffer, LG_MESSAGE,0);
            if(nb<=0) break;
            buffer[nb]='\0';

            if(strncmp(buffer,"win",3)==0){
                printf("Félicitations ! Vous avez trouvé le mot !\n");
                break;
            } else if(strncmp(buffer,"lost",4)==0){
                printf("Plus de vies ! Le mot était : %s\n", buffer+5);
                break;
            } else if(strncmp(buffer,"notfound",8)==0){
                sscanf(buffer,"notfound %d",&vies);
                printf("Lettre absente ! ");
                affichage(vies);
            } else {
                int v;
                char tmp[256];
                sscanf(buffer,"%s %d", tmp,&v);
                strcpy(motAffiche,tmp);
                vies=v;
                printf("Mot actuel : %s | ", motAffiche);
                affichage(vies);
            }
        }
    }

    close(sock);
    return 0;
}
