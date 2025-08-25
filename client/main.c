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
#define PATH_MAX 200
#define CLEAR_SCREEN() printf("\033[2J\033[H")

int main(int argc, char const *argv[])
{
    CLEAR_SCREEN();

    int sock, connectionStatus, bytes_read;
    char buffer[BUFFER_SIZE];
    socklen_t serverAddrLength;
    struct sockaddr_in serverAddr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);    
    serverAddrLength = sizeof(serverAddr);

    if (inet_pton(AF_INET, CONNECTION_HOST, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    connectionStatus = connect(sock, (struct sockaddr*) &serverAddr, serverAddrLength);

    if(connectionStatus == -1){
        fprintf(stderr, "(SERVER) Fail connect Server\n");
        exit(1);
    }

    printf("Connected to server %s:%d ✅\n", CONNECTION_HOST, PORT);

    snprintf(buffer, sizeof(buffer), "Hello World !!!");
    send(sock, buffer, BUFFER_SIZE, 0);

    snprintf(buffer, sizeof(buffer), "{notify: 'Bye Bye John'}");
    send(sock, buffer, BUFFER_SIZE, 0);

    while (connectionStatus == 0){
        bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        buffer[bytes_read] = '\0';
        if (strlen(buffer) > 0) {
            char cwd[PATH_MAX];
            const char *logo = "assets/zou.webp";

            getcwd(cwd, sizeof(cwd));
            snprintf(cwd, sizeof(cwd), "%s/%s", cwd, logo);

            notify_init("ZOU!");
            NotifyNotification *notif = notify_notification_new("Transport Région Sud", "LUKAS -> BUS n°836", logo);

            notify_notification_set_category(notif, "transport.bus");

            notify_notification_show(notif, NULL);
            g_object_unref(G_OBJECT(notif));
            notify_uninit();
        }

    
    }

    close(sock);    

    return 0;
}
 