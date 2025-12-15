#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include "game.h"

#define LG_MESSAGE 256

int main(int argc,char *argv[]){
    if(argc < 3){ printf("USAGE : %s ip port\n", argv[0]); return -1; }

    char ip[16]; strncpy(ip, argv[1], 16);
    int port; sscanf(argv[2], "%d", &port);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_aton(ip, &serv.sin_addr);

    if(connect(sock,(struct sockaddr*)&serv,sizeof(serv))<0){ perror("connect"); return -1; }

    char buffer[LG_MESSAGE];
    int nb = recv(sock, buffer, sizeof(buffer), 0);
    if(nb <= 0){ printf("Le serveur a fermé la connexion.\n"); return 0; }

    if(strcmp(buffer,"choose")==0){
        printf("Vous êtes joueur 1. Entrez le mot secret : ");
        fgets(buffer,sizeof(buffer),stdin);
        buffer[strcspn(buffer,"\n")]=0;
        send(sock, buffer, strlen(buffer)+1, 0);

        nb = recv(sock, buffer, sizeof(buffer), 0);
        printf("Début de la partie : %s\n", buffer);

        while(1){
            nb = recv(sock, buffer, sizeof(buffer), 0);
            if(nb <=0) break;
            if(strcmp(buffer,"win")==0){ printf("Joueur 2 a gagné !\n"); break; }
            else if(strncmp(buffer,"lost",4)==0){ printf("Joueur 2 a perdu. Mot : %s\n", buffer+5); break; }
            else { printf("Mot actuel : %s\n", buffer); }
        }
    }
    else if(strncmp(buffer,"start",5)==0){
        int tailleMot; sscanf(buffer,"start %d",&tailleMot);
        char motAffiche[MAX_WORD];
        for(int i=0;i<tailleMot;i++) motAffiche[i]='_';
        motAffiche[tailleMot]='\0';
        int vies=6;
        printf("Vous êtes joueur 2. Mot de %d lettres.\n", tailleMot);
        printf("Mot actuel : %s | Vies : %d\n", motAffiche,vies);

        while(1){
            printf("Lettre ou mot : ");
            fgets(buffer,sizeof(buffer),stdin);
            buffer[strcspn(buffer,"\n")]=0;
            send(sock, buffer, strlen(buffer)+1, 0);

            nb = recv(sock, buffer, sizeof(buffer), 0);
            if(nb <=0) break;

            if(sscanf(buffer,"notfound %d",&vies)==1){
                printf("Lettre absente ! Vies : %d\n",vies);
                affichage(vies);
            } else if(strncmp(buffer,"lost",4)==0){
                printf("Perdu ! Mot : %s\n",buffer+5);
                break;
            } else if(strcmp(buffer,"win")==0){
                printf("Victoire !\n");
                break;
            } else {
                char temp[MAX_WORD];
                int nv;
                if(sscanf(buffer,"%s %d",temp,&nv)==2){
                    strcpy(motAffiche,temp); vies=nv;
                    printf("Mot actuel : %s | Vies : %d\n", motAffiche,vies);
                } else printf("Message inconnu : %s\n",buffer);
            }
        }
    }

    close(sock);
    return 0;
}
