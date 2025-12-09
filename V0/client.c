#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "game.h"

int main(int argc, char *argv[]){
	int descripteurSocket;
	struct sockaddr_in sockaddrDistant;
	socklen_t longueurAdresse;

	char buffer[256];
	char reponse[256];
	int nb;

	char ip_dest[16];
	int port_dest;

	int vies;
	char motActuel[256];

	if(argc < 3){
		printf("USAGE : %s ip port\n", argv[0]);
		exit(-1);
	}

	strncpy(ip_dest, argv[1], 16);
	sscanf(argv[2], "%d", &port_dest);

	descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(descripteurSocket < 0){
		perror("Erreur en création de la socket...");
		exit(-1);
	}
	printf("Socket créée ! (%d)\n", descripteurSocket);

	longueurAdresse = sizeof(sockaddrDistant);
	memset(&sockaddrDistant, 0x00, longueurAdresse);

	sockaddrDistant.sin_family = AF_INET;
	sockaddrDistant.sin_port = htons(port_dest);
	inet_aton(ip_dest, &sockaddrDistant.sin_addr);

	if(connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1){
		perror("Erreur de connexion avec le serveur distant...");
		close(descripteurSocket);
		exit(-2);
	}
	printf("Connexion au serveur %s:%d réussie !\n", ip_dest, port_dest);

	nb = recv(descripteurSocket, reponse, sizeof(reponse), 0);
	if(nb <= 0){
		printf("Le serveur a fermé la connexion.\n");
		close(descripteurSocket);
		return 0;
	}

	int tailleMot;
	if(sscanf(reponse, "start %d", &tailleMot) != 1){
		printf("Protocole invalide.\n");
		close(descripteurSocket);
		return 0;
	}

	char motAffiche[256];
	for(int i=0; i<tailleMot; i++) motAffiche[i] = '_';
	motAffiche[tailleMot] = '\0';

	printf("Mot à deviner : %s\n", motAffiche);

	while(1){
		printf("\nEntrez une lettre/mot : ");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';


		nb = send(descripteurSocket, buffer, strlen(buffer)+1, 0);
		if(nb <= 0){
			printf("Erreur d'envoi ou serveur fermé.\n");
			break;
		}

		nb = recv(descripteurSocket, reponse, sizeof(reponse), 0);
		if(nb <= 0){
			printf("Le serveur a fermé la connexion.\n");
			break;
		}

		if (sscanf(reponse, "notfound %d", &vies) == 1) {
			printf("Lettre absente ! Il vous reste %d vies.\n", vies);
			affichage(vies);
		}

		else if (sscanf(reponse, "lost %s %d", motActuel, &vies) <= 1) {
			printf("Vous n'avez plus de vies ! Le mot était : %s\n", motActuel);
			break;
		}

		else if (strcmp(reponse, "win") == 0) {
			printf("Félicitations ! Vous avez trouvé le mot : %s\n", buffer);
			break;
		}

		else {
			// Cas "mot vies"
			// Exemple reçu : ab__d 4
			char motTemp[256];
			if (sscanf(reponse, "%s %d", motTemp, &vies) == 2) {
				strcpy(motAffiche, motTemp);
				printf("Mot actuel : %s | Vies restantes : %d\n", motAffiche, vies);
				affichage(vies);
			} else {
				printf("Message inconnu : %s\n", reponse);
			}
		}

	}

	close(descripteurSocket);
	return 0;
}
