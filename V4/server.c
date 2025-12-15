#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PORT 5003
#define LG_MESSAGE 256

// Fonction pour relayer les messages d'un joueur vers l'autre
void relai(int sockSrc, int sockDst) {
    char buffer[LG_MESSAGE];
    int nb;
    while((nb = recv(sockSrc, buffer, LG_MESSAGE, 0)) > 0) {
        send(sockDst, buffer, nb, 0);
    }
}

int main() {
    int sockListen = socket(AF_INET, SOCK_STREAM, 0);
    if(sockListen < 0){ 
        perror("socket"); 
        exit(-1); 
    }

    int opt = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addrLocal;
    memset(&addrLocal, 0, sizeof(addrLocal));
    addrLocal.sin_family = AF_INET;
    addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    addrLocal.sin_port = htons(PORT);

    if(bind(sockListen, (struct sockaddr *)&addrLocal, sizeof(addrLocal)) < 0){ 
        perror("bind");
        exit(-2); 
    }
    if(listen(sockListen, 5) < 0){ perror("listen"); exit(-3); }

    printf("Serveur PN V4 en écoute sur le port %d...\n", PORT);

    while(1) {
        struct sockaddr_in addr1, addr2;
        socklen_t l1 = sizeof(addr1), l2 = sizeof(addr2);

        printf("En attente de joueurs...\n");
        int player1 = accept(sockListen, (struct sockaddr *)&addr1, &l1);
        if(player1 < 0){ 
            perror("accept joueur1"); 
            continue; 
        }
        printf("Joueur 1 connecté\n");

        int player2 = accept(sockListen, (struct sockaddr *)&addr2, &l2);
        if(player2 < 0){ 
            perror("accept joueur2"); 
            close(player1); continue; 
        }
        printf("Joueur 2 connecté\n");

        // Envoyer simplement les rôles aux clients
        send(player1, "choose", 7, 0); // joueur 1 choisit le mot
        send(player2, "start", 6, 0);  // joueur 2 devine

        // Créer un processus pour relayer les messages
        pid_t pid = fork();
        if(pid == 0) {
            // Enfant : player1 -> player2
            relai(player1, player2);
            close(player1); close(player2);
            exit(0);
        } else {
            // Parent : player2 -> player1
            relai(player2, player1);
            close(player1); close(player2);
            wait(NULL);
        }

        printf("Fin de la partie.\n");
    }

    close(sockListen);
    return 0;
}
