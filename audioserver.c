#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "audio.h"

#define LOCALHOST "127.0.0.1"

int file_handler(char msg[]);

int main(int argc, char *argv[]){
    // 1) Ne prend pas de paramêtres en ligne de commande
    // 2) Attend que le client lui envoie une requête pour lire un fichier
    // 3) Lors de la réception : Ouvre un fichier et commence le transfert vers le client
    // 4) Quand le fichier est entierement lu, ferme le fichier et attend la prochaine requête
    // 5) Ne sert qu'un client à la fois
    // 6) Vérifie qu'il envoie les données à la bonne vitesse pour le client

    
    // SOCKET and BIND
    int newFd, err;
    struct sockaddr_in addr;
    socklen_t len, flen;
    struct sockaddr_in from;
    char msg[1024];
    char msg2[1024];
    newFd = socket(AF_INET,SOCK_DGRAM,0);
    if (newFd<0) { 
            perror("error creating socket");
            exit(1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    err = bind(newFd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    if (err<0) { 
            perror("error during bind");
            exit(1);
    }

    int stop = 0;
    while(stop == 0){
        // RECFROM
        flen = sizeof(struct sockaddr_in);
        len = recvfrom(newFd, msg, sizeof(msg), 0, (struct sockaddr*) &from, &flen);
        if (len<0) { 
                perror("error during recvfrom");
                exit(1);
        }
        printf("Received %d bytes from host %s port %d: %s\n", err, inet_ntoa(from.sin_addr), ntohs(from.sin_port), msg);
        int returnValueFile = file_handler(msg);
        // SENDTO
        if (strcmp(msg, "exit") == 0) {
            strcpy(msg2, "communication stopped\n");
            stop = 1;
        }
        else {
            strcpy(msg2, "Bien reçu\n");
        }
        
        err = sendto(newFd, msg2, strlen(msg2)+1, 0, (struct sockaddr*) &from, sizeof(struct sockaddr_in));
        if (err<0) { 
                perror("error during sendto");
                exit(1);
        }
    }

    close(newFd); 
    return 0;
}

int file_handler(char msg[]){
    int a, b, c;

    int *ptrA = &a;
    int *ptrB = &b;
    int *ptrC = &c; 
    char buffer[1024];

    int fd = aud_readinit (msg, ptrA, ptrB, ptrC); 

    if(fd == -1){
        perror("Error: readinit.");
        exit(1);
    }
    while(read(fd, &buffer, sizeof(buffer))>0){
       // return buffer;
    }
    read(fd, &buffer, sizeof(buffer));
    close(fd);
    return 0;
}