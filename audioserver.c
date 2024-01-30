#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "audio.h"

#define LOCALHOST "127.0.0.1"

int file_handler(char msg[]);

int main(int argc, char *argv[])
{
    // SOCKET and BIND
    int newFd, err;
    struct sockaddr_in addr;
    socklen_t len, flen;
    struct sockaddr_in from;
    char msg[1024];    // msg from client to server
    char msg2[1024];   // msg from server to client
    char buffer[1024]; // buffer to read the audio file, write on the audio system and play the music
    int tabInt[3];     // 3 integers sent to the client : sample_rate, sample_size, channels (the header of the audio file)
    newFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (newFd < 0)
    {
        perror("error creating socket");
        exit(1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    err = bind(newFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        perror("error during bind");
        exit(1);
    }

    // Loop until the client ask for exit
    while (1)
    {
        // RECFROM
        flen = sizeof(struct sockaddr_in);
        // Receive the name of the audio file from the client
        len = recvfrom(newFd, msg, sizeof(msg), 0, (struct sockaddr *)&from, &flen);
        if (len < 0)
        {
            perror("error during recvfrom");
            exit(1);
        }
        printf("Received %d bytes from host %s port %d: %s\n", err, inet_ntoa(from.sin_addr), ntohs(from.sin_port), msg);

        // Stop the communication if the client wants to
        if (strcmp(msg, "exit") == 0)
        {
            printf("Communication stopped\n");
            tabInt[0] = -1;
            tabInt[1] = -1;
            tabInt[2] = -1;
            err = sendto(newFd, tabInt, sizeof(tabInt), 0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
            if (err < 0)
            {
                perror("Error: during sendto");
                exit(1);
            }
            break;
        }

        int a, b, c; // sample_rate, sample_size, channels

        // Pointers to those parameters
        int *ptrA = &a;
        int *ptrB = &b;
        int *ptrC = &c;

        // Reads the header of the audio file and use the pointers to initialize the 3 parameters and put them on tabInt who will be sent to the client later
        int fd = aud_readinit(msg, ptrA, ptrB, ptrC);
        tabInt[0] = a;
        tabInt[1] = b;
        tabInt[2] = c;

        if (fd == -1)
        {
            strcpy(msg2, "Error: Le fichier n'a pas pu être chargé ou n'existe pas.");
            err = sendto(newFd, msg2, strlen(msg2) + 1, 0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
        }
        else
        {
            // Check the values of the header parameters
            for (int i = 0; i < 3; i++)
            {
                printf("%d\n", tabInt[i]);
            }

            // Send the header (tabInt) to the client
            err = sendto(newFd, tabInt, sizeof(tabInt), 0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
            if (err < 0)
            {
                perror("Error: during sendto");
                exit(1);
            }

            // Receive the acknowledgement from the client to start sending packets
            len = recvfrom(newFd, msg, sizeof(msg), 0, (struct sockaddr *)&from, &flen);
            if (len < 0)
            {
                perror("error during recvfrom after header was sended");
                exit(1);
            }

            // Loop until the EOF or if there is no acknowledgement from the client
            while (read(fd, &buffer, sizeof(buffer)) > 0 && strcmp(msg, "ok") == 0)
            {
                // Send packet per packet to the client
                err = sendto(newFd, buffer, sizeof(buffer), 0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
                if (err < 0)
                {
                    perror("Erreur decoupage à envoyer au client");
                    exit(1);
                }
                // Receive the acknowledgement from the client to continue sending packets
                len = recvfrom(newFd, msg, sizeof(msg), 0, (struct sockaddr *)&from, &flen);
                if (len < 0)
                {
                    perror("error during recvfrom after a packet was sended");
                    exit(1);
                }
            }
            printf("End of while");

            // End of the listening
            strcpy(msg2, "stop");
            err = sendto(newFd, msg2, strlen(msg2) + 1, 0, (struct sockaddr *)&from, sizeof(struct sockaddr_in));
            if (err < 0)
            {
                perror("Erreur envoi fin lecture fichier");
                exit(1);
            }
        }
        close(fd);
    }
    close(newFd);
    return 0;
}
