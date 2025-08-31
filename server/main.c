#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include "../src/cJSON.h"

#define PORT_SOCKET 8000
#define PORT_HTTP   8080
#define BUFFER_SIZE 4096
#define MAX_LENGTH_QUEUE 10
#define CLEAR_SCREEN() printf("\033[2J\033[H")

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[100];
int client_count = 0;

void broadcast_to_clients(const char *msg) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        send(clients[i], msg, strlen(msg), 0);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);

    pthread_mutex_lock(&clients_mutex);
    clients[client_count++] = client_sock;
    pthread_mutex_unlock(&clients_mutex);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while (1) {
        bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            if (bytes_read < 0) perror("recv");
            break;
        }
        buffer[bytes_read] = '\0';
    }

    close(client_sock);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_sock) {
            clients[i] = clients[--client_count];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Client disconnected\n");
    return NULL;
}

void *socket_init(void *arg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(EXIT_FAILURE); }

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT_SOCKET);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }

    if (listen(sock, MAX_LENGTH_QUEUE) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    printf("[TCP] server listening on port %d...\n", PORT_SOCKET);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t addr_size = sizeof(clientAddr);
        int clientSock = accept(sock, (struct sockaddr *)&clientAddr, &addr_size);
        if (clientSock < 0) { perror("accept"); continue; }

        printf("[TCP] New client from %s:%d 👤\n",
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port));

        int *new_sock = malloc(sizeof(int));
        *new_sock = clientSock;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, new_sock);
        pthread_detach(tid);
    }

    close(sock);
    return NULL;
}

const char *get_mime_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "text/plain";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    return "application/octet-stream";
}

void send_http_response(int client, const char *status, const char *content_type, const void *body, size_t body_len) {
    char header[BUFFER_SIZE];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n\r\n",
                              status, content_type, body_len);
    send(client, header, header_len, 0);
    send(client, body, body_len, 0);
}

void serve_file(int client, const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        const char *msg = "<h1>404 Not Found</h1>";
        send_http_response(client, "404 Not Found", "text/html", msg, strlen(msg));
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    send_http_response(client, "200 OK", get_mime_type(path), data, size);
    free(data);
}

void *http_server(void *arg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(EXIT_FAILURE); }

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_HTTP);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }

    if (listen(sock, MAX_LENGTH_QUEUE) < 0) {
        perror("listen"); exit(EXIT_FAILURE);
    }

    printf("[HTTP] server listening on port %d...\n", PORT_HTTP);

    while (1) {
        int client = accept(sock, NULL, NULL);
        if (client < 0) {perror("accept"); continue;}

        char buffer[BUFFER_SIZE];
        int len = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) {close(client); continue;}
        buffer[len] = '\0';

        char method[8], path[256];
        sscanf(buffer, "%s %s", method, path);

        if (strncmp(path, "/notify", 7) == 0) {
            char *msg = strstr(path, "card=");
            if (msg) {
                msg += 5;
                
                cJSON *json = cJSON_CreateObject();
                cJSON_AddStringToObject(json, "notify", msg);
                char *json_str = cJSON_Print(json);

                broadcast_to_clients(json_str);
                printf("[HTTP] Broadcast from HTTP: %s\n", json_str);
            }

            cJSON *json = cJSON_CreateObject();
            cJSON_AddStringToObject(json, "notify", "Zou");
            char *json_str = cJSON_Print(json);

            send_http_response(client, "200 OK", "text/json", json_str, strlen(json_str));
        } else if (strcmp(path, "/") == 0) {
            serve_file(client, "web/index.html");
        } else {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "./web/%s", path);
            serve_file(client, filepath);
        }

        close(client);
    }

    return NULL;
}

int main() {
    CLEAR_SCREEN();

    pthread_t t1, t2;
    pthread_create(&t1, NULL, socket_init, NULL);
    pthread_create(&t2, NULL, http_server, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
