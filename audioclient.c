#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "audio.h"
#include <signal.h>

static volatile int keepRunning = 1;
void intHandler(int dummy)
{
    keepRunning = 0;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, intHandler);  // For CTRL+C (if it get pressed, keepRunning will be 0 and the program will stop while warning the server)
    signal(SIGTSTP, intHandler); // For CTRL+Z

    // création de la socket pour envoyer le premier message de requête au serveur
    int fd;
    char msg[1024];    // msg from client to server
    char buffer[1024]; // buffer to read the audio file, write on the audio system and play the music
    int tabInt[3];     // 3 integer received from the server : sample_rate, sample_size, channels (the header of the audio file)
    int err;
    struct sockaddr_in dest;
    socklen_t len, flen;
    flen = sizeof(struct sockaddr_in);
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        perror("error creating socket");
        exit(1);
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(1234);
    dest.sin_addr.s_addr = inet_addr(argv[1]);

    // Loop until the server close the communication (when the client ask for exit)
    while (1)
    {
        strcpy(msg, argv[2]);
        // Send a msg to the server containing the filename to play
        err = sendto(fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
        if (err < 0)
        {
            perror("error during sendto");
            exit(1);
        }

        // Received from the server containing the 3 integer (the header) that we need to setup the listening of the audiofile
        len = recvfrom(fd, tabInt, sizeof(tabInt), 0, (struct sockaddr *)&dest, &flen);
        if (len < 0)
        {
            perror("error during recvfrom");
            exit(1);
        }

        // If the server got a problem to get the header of the audiofile
        if (tabInt[0] > 44100)
        {
            perror("Error while receiving the header, end of the communication.");
            exit(1);
        }

        // End of the communication without error, the client just asked to exit
        if (tabInt[0] == -1)
        {
            printf("End of the communication\n");
            break;
        }

        // Print the sample_rate, sample_size and channels to check if those are the same as the server
        for (int i = 0; i < 3; i++)
        {
            printf("Received %d bytes from host %s port %d: %d\n", err, inet_ntoa(dest.sin_addr), ntohs(dest.sin_port), tabInt[i]);
        }

        // FILTERS :
        // Speed of the music
        if (argv[3] != NULL && strcmp(argv[3], "speed") == 0)
        {
            tabInt[0] = tabInt[0] * atof(argv[4]);
        }
        // Stereo to mono
        if (argv[3] != NULL && strcmp(argv[3], "mono") == 0)
        {
            tabInt[2] = 1;
        }

        // Use of the header to setup the listening on the audio system
        int wtInit = aud_writeinit(tabInt[0], tabInt[1], tabInt[2]);
        if (wtInit == -1)
        {
            perror("Error: writeinit.");
            exit(1);
        }

        // Acknowledgment from the client to the server to allow him to start sending the packets
        strcpy(msg, "ok");
        err = sendto(fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
        if (err < 0)
        {
            perror("error during sendto after receiving header");
            exit(1);
        }

        // Beginning of the listening, packet per packet
        do
        {
            // Packet receive from the server, then put into the buffer
            len = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&dest, &flen);
            if (len < 0)
            {
                perror("error during recvfrom");
                exit(1);
            }
            else
            {
                // Filter: Volume
                if (argv[3] != NULL && strcmp(argv[3], "volume") == 0)
                {
                    for (int i = 0; i < sizeof(buffer); i++)
                    {
                        buffer[i] *= atof(argv[4]);
                    }
                }

                // Write the buffer on the audio system of the computer, to let the song get played
                write(wtInit, &buffer, sizeof(buffer));
                // Acknowledgment from the client to the server to allow him to continue sending packets
                strcpy(msg, "ok");
                if (len == 1024) // Check if this ackowledgment is during the listening phase, otherwise, at the end, it'll send "ok" to the server when he just want another audio file to open and it'll throw an error
                {
                    err = sendto(fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
                    if (err < 0)
                    {
                        perror("error during sendto after receiving a packet");
                        exit(1);
                    }
                }
            }
        } while (len == 1024 && keepRunning); // Loop while we are receiving packet from the server (if the server is reading the file, the packet will always be equal to 1024) or until the user press CTR+C or CTRL+Z
        close(wtInit);
        if (!keepRunning)
        {
            strcpy(msg, "not ok");
            err = sendto(fd, msg, strlen(msg) + 1, 0, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
            if (err < 0)
            {
                perror("error during sendto after ctrl+C or ctrl+Z");
                exit(1);
            }
            printf("\nL'utilisateur a interrompu le processus, fin du programme client.\n");
        }
        else
        {
            printf("Fin du morceau de musique, en espérant que vous avez passez un bon moment, à bientôt !\n");
        }
        break;
    }
    close(fd);
    return 0;
}