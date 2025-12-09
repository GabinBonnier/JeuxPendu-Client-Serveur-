#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "game.h"

#define LG_MESSAGE 256

int main(int argc, char *argv[]) {
    if(argc<3){ printf("USAGE: %s ip port\n", argv[0]); return -1; }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int sock;
    struct sockaddr_in addrServeur;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0){ perror("socket"); exit(-1); }

    memset(&addrServeur,0,sizeof(addrServeur));
    addrServeur.sin_family = AF_INET;
    addrServeur.sin_port = htons(port);
    inet_aton(ip, &addrServeur.sin_addr);

    if(connect(sock,(struct sockaddr*)&addrServeur,sizeof(addrServeur))<0){
        perror("connect"); exit(-2);
    }

    printf("Connecté au serveur %s:%d\n", ip, port);

    char msg[LG_MESSAGE];
    char motAffiche[50];
    int vies=6;
    while(1){
        int lus = recv(sock,msg,LG_MESSAGE,0);
        if(lus<=0){ printf("Connexion fermée par le serveur.\n"); break; }

        if(strncmp(msg,"WAIT",4)==0){
            printf("\nEn attente du tour de l'autre joueur...\n");
            for(int i=0;i<strlen(msg)-5;i++) motAffiche[i]='_';
            motAffiche[strlen(msg)-5]='\0';
            printf("Mot actuel : %s | Vies restantes : %d\n", motAffiche,vies);
        }
        else if(strncmp(msg,"YOUR_TURN",9)==0){
            int tailleMot;
            sscanf(msg,"YOUR_TURN %d",&tailleMot);
            for(int i=0;i<tailleMot;i++) motAffiche[i]='_';
            motAffiche[tailleMot]='\0';

            printf("\nC'est votre tour !\nMot actuel : %s | Vies restantes : %d\n", motAffiche,vies);
            printf("Entrez une lettre ou un mot : ");
            char buffer[LG_MESSAGE];
            fgets(buffer,sizeof(buffer),stdin);
            buffer[strcspn(buffer,"\n")]='\0';
            send(sock,buffer,strlen(buffer)+1,0);

            lus = recv(sock,msg,LG_MESSAGE,0);
            if(lus>0){
                if(strncmp(msg,"notfound",8)==0){
                    sscanf(msg,"notfound %d",&vies);
                    printf("Lettre absente ! Vies restantes : %d\n",vies);
                }
                else if(strncmp(msg,"win",3)==0){
                    printf("Félicitations, vous avez trouvé le mot !\n");
                    break;
                }
                else if(strncmp(msg,"lost",4)==0){
                    printf("Vous avez perdu ! Le mot était : %s\n",msg+5);
                    break;
                }
            }
        }
        else if(strncmp(msg,"END",3)==0){
            printf("%s\n",msg+4);
            break;
        }
    }

    close(sock);
    return 0;
}
