#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#define PORT 8000
#define BUFFER_SIZE 1024
#define MAX_LENGTH_QUEUE 2
#define CLEAR_SCREEN() printf("\033[2J\033[H")

void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    for (;;) {
        bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            if (bytes_read < 0) perror("recv");
            break;
        }
        buffer[bytes_read] = '\0';

        if (buffer[0] != '\0') {
            printf("Client says: %s\n", buffer);
        }

        if (strstr(buffer, "{notify:")) {
            ssize_t to_send = (ssize_t)strlen(buffer);
            ssize_t sent = send(client_sock, buffer, to_send, 0);
            if (sent < 0) perror("send");
        }
    }

    printf("Client disconnected\n");
    close(client_sock);
    return NULL;
}

int main(void) {
    CLEAR_SCREEN();

    signal(SIGPIPE, SIG_IGN);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (listen(sock, MAX_LENGTH_QUEUE) < 0) {
        perror("listen");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    puts("Waiting for clients...");

    for (;;) {
        struct sockaddr_in clientAddr;
        socklen_t addr_size = sizeof(clientAddr);
        memset(&clientAddr, 0, sizeof(clientAddr));

        int connectedSocket = accept(sock, (struct sockaddr *)&clientAddr, &addr_size);
        if (connectedSocket < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        printf("New client from %s:%d 👤\n",
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port));

        int *new_sock = malloc(sizeof(int));
        if (!new_sock) {
            fprintf(stderr, "malloc failed\n");
            close(connectedSocket);
            continue;
        }
        *new_sock = connectedSocket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, client_handler, new_sock) != 0) {
            perror("pthread_create");
            close(connectedSocket);
            free(new_sock);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(sock);
    return 0;
}
