#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "game.h"

#define PORT 5003
#define LG_MESSAGE 256

int main() {
    int listenSocket;
    struct sockaddr_in localAddress;

    // Create listening socket
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(listenSocket < 0){
        perror("socket");
        exit(-1);
    }
    printf("Listening socket created (%d)\n", listenSocket);

    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Initialize server address
    socklen_t addrLen = sizeof(localAddress);
    memset(&localAddress, 0, addrLen);
    localAddress.sin_family = AF_INET;
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddress.sin_port = htons(PORT);

    // Bind socket
    if(bind(listenSocket, (struct sockaddr *)&localAddress, addrLen) < 0){
        perror("bind");
        exit(-2);
    }

    // Listen pour incoming connections
    if(listen(listenSocket, 5) < 0){
        perror("listen");
        exit(-3);
    }

    printf("Server ready, waiting pour players on port %d...\n", PORT);

    // Main server loop -> always waiting pour new game sessions
    while(1) {

        printf("\nAttends le joueur 1\n");
        struct sockaddr_in addr1;
        socklen_t l1 = sizeof(addr1);
        int player1 = accept(listenSocket, (struct sockaddr *)&addr1, &l1);
        if(player1 < 0){
            perror("accept");
            continue;
        }
        printf("Joueur 1 connecté.\n");

        printf("Attends le joueur 2.\n");        
        struct sockaddr_in addr2;
        socklen_t l2 = sizeof(addr2);
        int player2 = accept(listenSocket, (struct sockaddr *)&addr2, &l2);
        if(player2 < 0){
            perror("accept");
            close(player1);
            continue;
        }
        printf("Joueur 2 connecté.\n");

        printf("Serveur crée pour la game\n");

        // Create a child process to handle this game session
        pid_t pid = fork();

        if(pid < 0){
            close(player1);
            close(player2);
            continue;
        }

        // child process
        if(pid == 0){

             // child does not use the listening socket
            close(listenSocket);

            Game game;
            char secretWord[50];

            // Ask joueur 1 to choose the secret word
            send(player1, "choose", 7, 0);
            memset(secretWord, 0, sizeof(secretWord));
            recv(player1, secretWord, sizeof(secretWord), 0);
            secretWord[strcspn(secretWord, "\n")] = 0;

            init_game(&game, secretWord);

            char word_global[50];
            strcpy(word_global, game.secret_word);

            // Notify both players that the game is starting
            char message[LG_MESSAGE];
            sprintf(message, "start %ld", strlen(word_global));
            send(player1, message, strlen(message)+1, 0);
            send(player2, message, strlen(message)+1, 0);

            // Create the hidden word with -
            char discovered[50];
            for(int i=0; i<strlen(word_global); i++){
                discovered[i] = '_';
            }
            discovered[strlen(word_global)] = '\0';

            int letter_not_found = strlen(word_global);

            // Game loop
            while(1){

                //Receive guess from player 2
                char received[LG_MESSAGE];
                memset(received, 0, LG_MESSAGE);

                int nb = recv(player2, received, LG_MESSAGE, 0);
                if(nb <= 0){
                    printf("Serveur %d joueur 2 déconnecté.\n",pid);
                    break;
                }

                int tab[50];
                test_input_game(&game, received, tab);

                // Letter not found
                if(tab[0] == -1){
                    game.nb_life--;
                    if(game.nb_life <= 0){
                        sprintf(message, "lost %s", word_global);
                        send(player1, message, strlen(message)+1, 0);
                        send(player2, message, strlen(message)+1, 0);
                        break;
                    }
                    sprintf(message, "notfound %d", game.nb_life);
                }
                // Whole word guessed
                else if(tab[0] == 100){
                    sprintf(message, "win");
                    send(player1, message, strlen(message)+1, 0);
                    send(player2, message, strlen(message)+1, 0);
                    break;
                }
                // Correct letter(s)
                else {
                    int found = 0;
                    for(int i=1; i<=tab[0]; i++){
                        if(discovered[tab[i]] == '_'){
                            discovered[tab[i]] = word_global[tab[i]];
                            found++;
                        }
                    }
                    letter_not_found -= found;

                    if(letter_not_found == 0){
                        sprintf(message, "win");
                    } else {
                        sprintf(message, "%s %d", discovered, game.nb_life);
                    }
                }

                // Send update to both players
                send(player1, message, strlen(message)+1, 0);
                send(player2, message, strlen(message)+1, 0);
            }

            // Close client sockets in the child
            close(player1);
            close(player2);
            
            printf("Serveur %d Fin de la session\n",pid);
            exit(0);
        }

        close(player1);
        close(player2);
    }

    close(listenSocket);
    return 0;
}
