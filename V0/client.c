#include <stdio.h>
#include <stdlib.h> /* pour exit */
#include <unistd.h> /* pour read, write, close, sleep */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h> /* pour memset */
#include <netinet/in.h> /* pour struct sockaddr_in */
#include <arpa/inet.h> /* pour htons et inet_aton */

int main(int argc, char *argv[]){
	int descripteurSocket;
	struct sockaddr_in sockaddrDistant;
	socklen_t longueurAdresse;

	char buffer[256];
	char reponse[256];
	int nb;

	char ip_dest[16];
	int port_dest;

	/* Vérification arguments */
	if(argc < 3){
		printf("USAGE : %s ip port\n", argv[0]);
		exit(-1);
	}

	strncpy(ip_dest, argv[1], 16);
	sscanf(argv[2], "%d", &port_dest);

	/* ------------ Création socket ------------ */
	descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(descripteurSocket < 0){
		perror("Erreur en création de la socket...");
		exit(-1);
	}
	printf("Socket créée ! (%d)\n", descripteurSocket);

	/* ------------ Remplissage sockaddr_in ------------ */
	longueurAdresse = sizeof(sockaddrDistant);
	memset(&sockaddrDistant, 0x00, longueurAdresse);

	sockaddrDistant.sin_family = AF_INET;
	sockaddrDistant.sin_port = htons(port_dest);
	inet_aton(ip_dest, &sockaddrDistant.sin_addr);

	/* ------------ Connexion ------------ */
	if(connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1){
		perror("Erreur de connexion avec le serveur distant...");
		close(descripteurSocket);
		exit(-2);
	}
	printf("Connexion au serveur %s:%d réussie !\n", ip_dest, port_dest);


	/* ===========================================================
	               LECTURE DU MESSAGE "start x"
	   =========================================================== */

	nb = recv(descripteurSocket, reponse, sizeof(reponse), 0);
	if(nb <= 0){
		printf("Le serveur a fermé la connexion.\n");
		close(descripteurSocket);
		return 0;
	}
	printf("Serveur : %s\n", reponse);


	/* ===========================================================
	                     BOUCLE DE JEU
	   =========================================================== */
	while(1){
		printf("\nEntrez une lettre : ");
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strcspn(buffer, "\n")] = '\0';

		/* --- Modification minimale : une lettre obligatoire --- */
		if(strlen(buffer) != 1){
			printf("Veuillez entrer UNE SEULE lettre.\n");
			continue;
		}

		/* ----------- Envoi au serveur ----------- */
		nb = send(descripteurSocket, buffer, strlen(buffer)+1, 0);
		if(nb == -1){
			perror("Erreur en écriture...");
			break;
		}
		if(nb == 0){
			printf("Socket fermée par le serveur.\n");
			break;
		}

		printf("Message \"%s\" envoyé (%d octets)\n", buffer, nb);

		/* ----------- Réception réponse ----------- */
		nb = recv(descripteurSocket, reponse, sizeof(reponse), 0);
		if(nb == -1){
			perror("Erreur en lecture...");
			break;
		}
		if(nb == 0){
			printf("Le serveur a fermé la connexion.\n");
			break;
		}

		printf("Réponse du serveur : %s\n", reponse);

		/* --- Arrêt sur fin de partie (win / lose) --- */
		if(strncmp(reponse, "win", 3) == 0 ||
		   strncmp(reponse, "lose", 4) == 0){
			printf("Fin de la partie.\n");
			break;
		}
	}

	close(descripteurSocket);
	return 0;
}
