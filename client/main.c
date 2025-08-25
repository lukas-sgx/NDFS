#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libnotify/notify.h>

#define PORT 8000
#define CONNECTION_HOST "192.168.86.83"
#define BUFFER_SIZE 1024
#define CLEAR_SCREEN() printf("\033[2J\033[H")

int main(int argc, char const *argv[])
{
    CLEAR_SCREEN();

    int sock, connectionStatus, bytes_read;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLength = sizeof(serverAddr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(EXIT_FAILURE); }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, CONNECTION_HOST, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    connectionStatus = connect(sock, (struct sockaddr*)&serverAddr, serverAddrLength);
    if (connectionStatus == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server %s:%d ✅\n", CONNECTION_HOST, PORT);

    snprintf(buffer, sizeof(buffer), "Hello World !!!");
    send(sock, buffer, strlen(buffer), 0);

    snprintf(buffer, sizeof(buffer), "{notify: 'Bye Bye John'}");
    send(sock, buffer, strlen(buffer), 0);

    char cwd[PATH_MAX];
    
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    const char *logo = "assets/zou.webp";

    char full_path[PATH_MAX];
    int n = snprintf(full_path, sizeof(full_path), "%s/%s", cwd, logo);
    if (n >= sizeof(full_path)) {
        exit(EXIT_FAILURE);
    }

    printf("m: %s\n", full_path);

    while (1) {
        bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) break;
        buffer[bytes_read] = '\0';

        if (strlen(buffer) > 0) {
            notify_init("ZOU!");

            NotifyNotification *notif = notify_notification_new(
                "Transport Région Sud",
                "LUKAS -> BUS n°836",
                full_path
            );

            notify_notification_show(notif, NULL);
            g_object_unref(G_OBJECT(notif));
            notify_uninit();
        }
    }

    close(sock);
    return 0;
}