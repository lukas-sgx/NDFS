#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8000
#define CONNECTION_HOST "192.168.86.83"
#define BUFFER_SIZE 1024
#define CLEAR_SCREEN() printf("\033[2J\033[H")

int main(int argc, char const *argv[])
{
    CLEAR_SCREEN();

    int sock, inet, connectionStatus;
    char buffer[BUFFER_SIZE];
    __u_long serverAddrLength;
    struct sockaddr_in serverAddr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = PORT;    
    serverAddrLength = sizeof(serverAddr);

    inet = inet_pton(AF_INET, CONNECTION_HOST, &serverAddr.sin_addr);

    connectionStatus = connect(sock, (struct sockaddr*) &serverAddr, serverAddrLength);

    if(connectionStatus == -1){
        fprintf(stderr, "(SERVER) Fail connect Server\n");
        exit(1);
    }

    while (connectionStatus > 0){
        
    }

    close(sock);    

    return 0;
}
 