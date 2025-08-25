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

void *client_handler(void *arg){
    int client_sock = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        if (strlen(buffer) > 0) {
            printf("Client says: %s\n", buffer);
        }


        if(strstr(buffer, "{notify:")){
            send(client_sock, buffer, strlen(buffer), 0);
        }
    }

    printf("Client disconnected\n");
    close(client_sock);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int server_sock, *new_sock;
    int sock, b;
    char buffer[BUFFER_SIZE];
    socklen_t serverAddrLength;
    struct sockaddr_in serverAddr, clientAddr;

    CLEAR_SCREEN();

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT); 
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

        printf("New victim from %s:%d 👤\n",
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        
        pthread_t thread_id;
        new_sock = malloc(sizeof(int));
        *new_sock = connectedSocket;

        if (pthread_create(&thread_id, NULL, client_handler, new_sock) != 0) {
            perror("pthread_create");
            close(connectedSocket);
            free(new_sock);
        }

        pthread_detach(thread_id);
    }

    close(sock);

    return 0;
}
