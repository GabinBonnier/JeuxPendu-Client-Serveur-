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

    //Check command line arguments
    if(argc < 3){
        printf("USAGE : %s ip port\n", argv[0]);
        exit(-1);
    }

    //Store server IP and port
    char ip_dest[16];
    int port_dest;
    strncpy(ip_dest, argv[1], 16);
    sscanf(argv[2], "%d", &port_dest);

    int sock;
    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){ perror("Socket creation failed"); exit(-1); }

    //Initialize server address structure
    memset(&serverAddr, 0, addrLen);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port_dest);
    inet_aton(ip_dest, &serverAddr.sin_addr);

    //Connect to server
    if(connect(sock, (struct sockaddr *)&serverAddr, addrLen) == -1){
        perror("Connection failed");
        close(sock);
        exit(-2);
    }
    printf("Connexion au serveur %s:%d réussie !\n", ip_dest, port_dest);

    char buffer[LG_MESSAGE];
    int nb = recv(sock, buffer, sizeof(buffer), 0);
    if(nb <= 0){
        printf("Le serveur a fermé la connexion.\n");
        close(sock);
        return 0;
    }

    //Determine player role
    if(strcmp(buffer, "choose") == 0){
        //Player 1: choose a secret word
        printf("Vous êtes le joueur 1. Entrez le mot secret pour le joueur 2 : ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        send(sock, buffer, strlen(buffer)+1, 0);

        //Wait for start message from server
        nb = recv(sock, buffer, sizeof(buffer), 0);
        if(nb <= 0){
            printf("Le serveur a fermé la connexion.\n");
            close(sock);
            return 0;
        }
        printf("Début de la partie : %s\n", buffer);

        //Player 1 observes the game
        while(1){
            nb = recv(sock, buffer, sizeof(buffer), 0);
            if(nb <= 0){ printf("Le serveur a fermé la connexion.\n"); break; }

            if(strcmp(buffer, "win") == 0){
                printf("Le joueur 2 a gagné la partie !\n");
                break;
            } else if(strncmp(buffer, "lost", 4) == 0){
                printf("Le joueur 2 a perdu la partie. %s\n", buffer+5);
                break;
            } else {
                //Show current progress
                printf("Mot actuel : %s\n", buffer);
            }
        }

    } else if(strncmp(buffer, "start", 5) == 0){
        //Player 2: start guessing loop
        int tailleMot;
        if(sscanf(buffer, "start %d", &tailleMot) != 1){
            printf("Protocole invalide du serveur.\n");
            close(sock);
            return 0;
        }

        //Initialize guessed word with -
        char motAffiche[256];
        for(int i=0; i<tailleMot; i++) motAffiche[i] = '_';
        motAffiche[tailleMot] = '\0';

        int vies = 6;
        printf("Vous êtes le joueur 2. Le mot comporte %d lettres.\n", tailleMot);
        printf("Mot actuel : %s\n", motAffiche);

        //Game loop
        while(1){
            printf("\nEntrez une lettre ou le mot complet : ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';

            //Send answer to server
            send(sock, buffer, strlen(buffer)+1, 0);

            //Receive update from server
            nb = recv(sock, buffer, sizeof(buffer), 0);
            if(nb <= 0){
                printf("Le serveur a fermé la connexion.\n");
                break;
            }

            //Letter not found -> lose a life
            if(sscanf(buffer, "notfound %d", &vies) == 1){
                printf("Lettre absente ! Vies restantes : %d\n", vies);
				affichage(vies);
            } 
            //Player lost -> show correct word
            else if(sscanf(buffer, "lost %s", motAffiche) == 1){
                printf("Plus de vies ! Le mot était : %s\n", motAffiche);
                break;
            } 
            //Player won
            else if(strcmp(buffer, "win") == 0){
                printf("Félicitations ! Vous avez trouvé le mot !\n");
                break;
            } 
            //Update word display
            else {
                char temp[256];
                if(sscanf(buffer, "%s %d", temp, &vies) == 2){
                    strcpy(motAffiche, temp);
                    printf("Mot actuel : %s | Vies restantes : %d\n", motAffiche, vies);
                } else {
                    printf("Message inconnu : %s\n", buffer);
                }
            }
        }

    } else {
        printf("Message initial inconnu du serveur : %s\n", buffer);
    }

    //Close socket
    close(sock);
    return 0;
}
