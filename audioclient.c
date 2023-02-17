#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "audio.h"

#define LOCALHOST "127.0.0.1"

int main(int argc, char *argv[]){
    // 1) Envoi une requête au serveur contenant le nom du fichier qu'il souhaite jouer
    // 2) Recoit le flux de données du serveur et il joue le son pendant qu'il reçoit les données

    //création de la socket pour envoyer le premier message de requête au serveur
    int fd;
    char msg[1024];
    char msg2[1024];
    int err;
    struct sockaddr_in dest;
    socklen_t len, flen;
    flen = sizeof(struct sockaddr_in);
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (fd<0) { 
        perror("error creating socket");
        exit(1);
     }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(1234);
    dest.sin_addr.s_addr = inet_addr(LOCALHOST);

    while(strcmp(msg2, "communication stopped\n") != 0){
        printf("Which song do you want to play ?\n");
        scanf("%s", msg);
        err = sendto(fd, msg, strlen(msg)+1, 0, (struct sockaddr*) &dest, sizeof(struct sockaddr_in));
        
        if (err<0) { 
            perror("error during sendto");
            exit(1);
        }

        // Attente de la réponse serveur + réponse serveur
        len = recvfrom(fd, msg2, sizeof(msg2), 0, (struct sockaddr*) &dest, &flen);
        if (len<0) { 
            perror("error during recvfrom");
            exit(1);
        }
        printf("Received %d bytes from host %s port %d: %s\n", err, inet_ntoa(dest.sin_addr), ntohs(dest.sin_port), msg2);
    }
    

    close(fd);
    
    return 0;
}