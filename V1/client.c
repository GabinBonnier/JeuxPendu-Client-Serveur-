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
    if(argc < 3) {
        printf("USAGE : %s ip port\n", argv[0]);
        exit(-1);
    }
    char *ip_dest = argv[1];
    int port_dest = atoi(argv[2]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0){ perror("socket"); exit(-1); }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_dest);
    inet_aton(ip_dest,&addr.sin_addr);
    if(connect(sock,(struct sockaddr*)&addr,sizeof(addr))<0){
        perror("connect");
        close(sock);
        exit(-2);
    }
    printf("Connecté au serveur.\n");
    char buffer[LG_MESSAGE];
    int nb = recv(sock, buffer, LG_MESSAGE, 0);
    if(nb <=0){ printf("Le serveur a fermé la connexion.\n"); return 0; }
    int tailleMot;
    if(sscanf(buffer,"start %d",&tailleMot)!=1){
        printf("Protocole invalide.\n"); close(sock); return 0;
    }
    char motAffiche[50];
    for(int i=0;i<tailleMot;i++) motAffiche[i]='_';
    motAffiche[tailleMot]='\0';
    printf("Le mot comporte %d lettres.\nMot à deviner : %s\n", tailleMot, motAffiche);
    
    while(1){
        int nb = recv(sock, buffer, LG_MESSAGE, 0);
        if(nb <= 0){
            printf("Le serveur a fermé la connexion.\n");
            break;
        }
        buffer[nb] = '\0';
        
        if(strcmp(buffer, "YOUR_TURN") == 0){
            printf("\nC'est votre tour !\n Entrez une lettre/mot : ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            send(sock, buffer, strlen(buffer)+1, 0);
            
            nb = recv(sock, buffer, LG_MESSAGE, 0);
            if(nb <= 0){
                printf("Le serveur a fermé la connexion.\n");
                break;
            }
            buffer[nb] = '\0';
            
            if(strncmp(buffer, "notfound", 8) == 0){
                int vies;
                sscanf(buffer, "notfound %d", &vies);
                printf("Lettre absente ! Il vous reste %d vies.\n", vies);
                affichage(vies);
            }
            else if(strncmp(buffer, "lost", 4) == 0){
                char mot[50];
                sscanf(buffer, "lost %s", mot);
                printf("Vous n'avez plus de vies ! Le mot était : %s\n", mot);
                break;
            }
            else if(strncmp(buffer, "win", 3) == 0){
                char mot[50];
                sscanf(buffer, "win %s", mot);
                printf("Félicitations ! Vous avez trouvé le mot : %s\n", mot);
                break;
            }
            else {
                int vies;
                char motTemp[50];
                if(sscanf(buffer, "%s %d", motTemp, &vies) == 2){
                    strcpy(motAffiche, motTemp);
                    printf("Mot actuel : %s | Vies restantes : %d\n", motAffiche, vies);
                    affichage(vies);
                }
            }
        } 
        else if(strncmp(buffer, "UPDATE", 6) == 0){
            char motTemp[50];
            sscanf(buffer, "UPDATE %s", motTemp);
            strcpy(motAffiche, motTemp);
            printf("Ce n'est pas votre tour ! Patientez...\n");
            printf("Mot actuel : %s\n", motAffiche);
        }
        else if(strncmp(buffer, "END", 3) == 0){
            printf("%s\n", buffer+4);
            break;
        }
    }
    close(sock);
    return 0;
}