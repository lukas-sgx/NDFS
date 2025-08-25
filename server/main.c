#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define MAX_LENGTH_QUEUE 2
#define CLEAR_SCREEN() printf("\033[2J\033[H")



int main(int argc, char const *argv[])
{
    int sock, b;
    char buffer[BUFFER_SIZE];
    __u_long serverAddrLength;
    struct sockaddr_in serverAddr, clientAddr;

    CLEAR_SCREEN();

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = PORT; 
    serverAddr.sin_addr.s_addr = INADDR_ANY;    
    serverAddrLength = sizeof(serverAddr);

    b = bind(sock, (struct sockaddr*) &serverAddr, serverAddrLength);

    if(b == -1){
        fprintf(stderr, "(SERVER) Echec de liaison pour le socket\n");
        exit(1);
    }

    if(listen(sock, MAX_LENGTH_QUEUE) == -1){
        fprintf(stderr, "(SERVER) Fail listen socket\n");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);
    puts("Wait devices...");

    while (1)
    {
        int connectedSocket = accept(sock, (struct sockaddr*) &serverAddr, (socklen_t *) &serverAddrLength);

        printf("New connection from %s:%d\n",
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        
    }

    close(sock);

    return 0;
}
