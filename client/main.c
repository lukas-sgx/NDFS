#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libnotify/notify.h>
#include "../src/cJSON.h"

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

    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }



    while (1) {
        bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) break;
        buffer[bytes_read] = '\0';

        if (strlen(buffer) > 0) {

            cJSON *json = cJSON_Parse(buffer);
            if (json == NULL) {
                const char *error_ptr = cJSON_GetErrorPtr();
                if   (error_ptr != NULL) {
                    printf("Error: %s\n", error_ptr);
                }
                return 1;
            }

            cJSON *notify_item = cJSON_GetObjectItemCaseSensitive(json, "notify");
            if (cJSON_IsString(notify_item) && (notify_item->valuestring != NULL)) {
                printf("Notify: %s\n", notify_item->valuestring);
            }

            char logo[1024], name[50], desc[150], info[150];

            if(strcmp(notify_item->valuestring, "Zou") == 1){
                snprintf(logo, sizeof(logo), "assets/zou.webp");
                snprintf(desc, sizeof(desc), "Transport Région Sud");
                snprintf(name, sizeof(name), "ZOU!");
                snprintf(info, sizeof(info), "LUKAS -> BUS n°836");
            } else if(strcmp(notify_item->valuestring, "LigneAzur") == 1){
                snprintf(logo, sizeof(logo), "assets/azur.png");
                snprintf(desc, sizeof(desc), "Transport Lignes Azur");
                snprintf(name, sizeof(name), "Lignes Azur");
                snprintf(info, sizeof(info), "LUKAS -> TRAM n°3");
            }  

            cJSON_Delete(json);

        
            char full_path[PATH_MAX];
            int n = snprintf(full_path, sizeof(full_path), "%s/%s", cwd, logo);
            if (n >= sizeof(full_path)) {
                exit(EXIT_FAILURE);
            }

            notify_init(name);

            NotifyNotification *notif = notify_notification_new(
                desc,
                info,
                full_path
            );

            notify_notification_show(notif, NULL);
            g_object_unref(G_OBJECT(notif));
            notify_uninit();
            cJSON_Delete(json);
        }
    }

    close(sock);
    return 0;
}