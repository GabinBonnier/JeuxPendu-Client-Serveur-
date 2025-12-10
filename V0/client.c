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

	int descripteurSocket;//Descripteur of the socket to comunicate with client
	struct sockaddr_in sockaddrDistant; //Structure containing the server's IP address and port
	socklen_t longueurAdresse;//Len of sock_addr

	char buffer[256];//Got the intput of the player in order to be send to the server
	char reponse[256];//Got the reponse of the client
	int nb;//Number of bytes of the sending and the receiving

	char ip_dest[16];//Ip of the server
	int port_dest;//Port of the server

	int vies;//Life of the player
	char motActuel[256];//Word return by the server if the player lost

	//Connection with a server

	//Test the port
	if(argc < 3){
		printf("USAGE : %s ip port\n", argv[0]);
		exit(-1);
	}

	//Set the ip
	strncpy(ip_dest, argv[1], 16);
	sscanf(argv[2], "%d", &port_dest);

	// Create a socket
	descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(descripteurSocket < 0){
		perror("Erreur en création de la socket...");
		exit(-1);
	}
	printf("Socket créée ! (%d)\n", descripteurSocket);

	//Set the len of the address
	longueurAdresse = sizeof(sockaddrDistant);
	memset(&sockaddrDistant, 0x00, longueurAdresse);

	sockaddrDistant.sin_family = AF_INET;
	sockaddrDistant.sin_port = htons(port_dest);
	inet_aton(ip_dest, &sockaddrDistant.sin_addr);

	//-1 is return by the socket if the connexion failed
	if(connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1){
		perror("Erreur de connexion avec le serveur distant...");
		close(descripteurSocket);
		exit(-2);
	}
	printf("Connexion au serveur %s:%d réussie !\n", ip_dest, port_dest);
	
	//Test the connexion
	nb = recv(descripteurSocket, reponse, sizeof(reponse), 0);
	if(nb <= 0){
		printf("Le serveur a fermé la connexion.\n");
		close(descripteurSocket);
		return 0;
	}

	//Set the len of the word and print it
	int tailleMot;
	if(sscanf(reponse, "start %d", &tailleMot) != 1){
		printf("Protocole invalide.\n");
		close(descripteurSocket);
		return 0;
	}
	printf("Le mot comporte %d lettres. \n", tailleMot);

	//Show the word in -
	char motAffiche[256];
	for(int i=0; i<tailleMot; i++) motAffiche[i] = '_';
	motAffiche[tailleMot] = '\0';

	printf("Mot à deviner : %s\n", motAffiche);

	while(1){

		//Receive the intput of the user
		printf("\nEntrez une lettre/mot : ");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';

		//Send the answer at the server
		nb = send(descripteurSocket, buffer, strlen(buffer)+1, 0);
		if(nb <= 0){
			printf("Erreur d'envoi ou serveur fermé.\n");
			break;
		}

		//Test the connexion
		nb = recv(descripteurSocket, reponse, sizeof(reponse), 0);
		if(nb <= 0){
			printf("Le serveur a fermé la connexion.\n");
			break;
		}

		//The server send a lost message
		if (sscanf(reponse, "notfound %d", &vies) == 1) {
			printf("Lettre absente ! Il vous reste %d vies.\n", vies);
			affichage(vies);
		}

		//The server send a lost message -> endgame
		else if (sscanf(reponse, "lost %s", motActuel) == 1) {
			printf("Vous n'avez plus de vies ! Le mot était : %s\n", motActuel);
			break;
		}

		//THe server send a win message -> word send was good
		else if (strcmp(reponse, "win") == 0) {
			printf("Félicitations ! Vous avez trouvé le mot : %s\n", buffer);
			break;
		}

		//Show the word with the letter find and the - if the letter was good
		else {
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
