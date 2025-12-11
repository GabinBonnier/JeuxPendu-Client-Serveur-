#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5003
#define LG_MESSAGE 256

#include "game.h"

int main() {
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse = sizeof(pointDeRencontreLocal);

    // Création de la socket d'écoute
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if(socketEcoute < 0){
        perror("socket");
        exit(-1);
    }
    printf("Socket créée ! (%d)\n", socketEcoute);

    int opt = 1;
    setsockopt(socketEcoute, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = AF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    if(bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0){
        perror("bind");
        exit(-2);
    }

    if(listen(socketEcoute, 5) < 0){
        perror("listen");
        exit(-3);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    // Infinite loop while no connection client
    while(1) {
        int socketDialogue;
        struct sockaddr_in pointDeRencontreDistant;
        socklen_t tailleDistant = sizeof(pointDeRencontreDistant);

        printf("\nEn attente d'un nouveau client...\n");
        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &tailleDistant);
        if(socketDialogue < 0){
            perror("accept");
            continue;
        }
        //Show the connection with client and the ip of a client 
        printf("Client connecté : %s:%d\n",
               inet_ntoa(pointDeRencontreDistant.sin_addr),
               ntohs(pointDeRencontreDistant.sin_port));

        // Initialisation of the game element's
        Game game;
        init_game(&game);
        char word_global[50];
		printf("Le mot comporte %d lettres. \n", (int)strlen(game.secret_word));
        strcpy(word_global, game.secret_word);
        //Show the secret word
        printf("Mot choisi : %s\n", word_global);
        char reponse[LG_MESSAGE];
        sprintf(reponse, "start %ld", strlen(word_global));
        send(socketDialogue, reponse, strlen(reponse)+1, 0);
        
        //Tab who contains the word in - and the letter who was find
        char lettresTrouvees[50];
        for(int i=0; i<strlen(word_global); i++) lettresTrouvees[i] = '_';
        lettresTrouvees[strlen(word_global)] = '\0';
        
        //Number of letter who are not find yet
        int lettresRestantes = strlen(word_global);
        //Message from the client socket
        char messageRecu[LG_MESSAGE];

        // Game loop
        while(1){
            
            //Test the connection
            memset(messageRecu, 0, LG_MESSAGE);
            int lus = recv(socketDialogue, messageRecu, LG_MESSAGE, 0);
            if(lus <= 0){
                printf("Client déconnecté.\n");
                break;
            }

            //Tab contains the occurences of a letter choose by the client
            //Or -1 for lost and -100 for the win
            int tab[50];
            //Test the answer
            test_input_game(&game, messageRecu, tab);

            //Erreur du client in the game -> -1 life /6
            if(tab[0] == -1){
                game.nb_life--;
                if(game.nb_life <= 0){
                    sprintf(reponse, "lost %s", word_global);
                    send(socketDialogue, reponse, strlen(reponse)+1, 0);
                    printf("Joueur perdant, plus de vies.\n");
                    break;
                }
                sprintf(reponse, "notfound %d", game.nb_life);
            }
            //Win of the client
            else if(tab[0] == 100){
                sprintf(reponse, "win");
            }
            //Letter is good -> modification of lettres_trouves
            else {
                for(int i=1; i<=tab[0]; i++){
                    lettresTrouvees[tab[i]] = word_global[tab[i]];
                }
                lettresRestantes -= tab[0];
                sprintf(reponse, "%s %d", lettresTrouvees, game.nb_life);
            }

            //If the all of the letter is found -> win
            if(lettresRestantes == 0){
                sprintf(reponse, "win");
                printf("Mot trouvé par le joueur.\n");
            }

            //Send the result of the letter at the client
            send(socketDialogue, reponse, strlen(reponse)+1, 0);

            //The socket send a win message
            if(strcmp(reponse, "win") == 0){
                printf("Joueur gagnant !\n");
                break;
            }

        }
        close(socketDialogue);
        printf("Fin de la session client.\n");
    }

    close(socketEcoute);
    return 0;
}
