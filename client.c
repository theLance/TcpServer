#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include "defines.h"


void process_package(int tcpSocket)
{
    for (;;) {
        char buff[RW_BUFFER_MAX];
        bzero(buff, sizeof(buff));

        // Read string
        printf("String to send to server: ");
        int n = 0;
        while ((buff[n++] = getchar()) != '\n');

        // Send string
        write(tcpSocket, buff, sizeof(buff));

        // Receive reply
        bzero(buff, sizeof(buff));
        read(tcpSocket, buff, sizeof(buff));
        printf("String received from server: %s", buff);
    }
}

int main()
{
    // The socket on which we will communicate.
    int tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1) {
        printf("Could not create TCP socket.\n");
        exit(1);
    }
    else {
        printf("TCP socket created successfully.\n");
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // The port, starting from default and trying a few thereafter.
    int connect_result = -1;
    int port = DEFAULT_PORT;
    for(; connect_result != 0 && port < DEFAULT_PORT + 100; ++port) {
        servaddr.sin_port = htons(port);
        connect_result = connect(tcpSocket, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if(connect_result != 0) {
            printf("Failed to connect socket on port %d...\n", port);
        }
    }
    if (connect_result != 0) {
        printf("Cannot connect to server. Shutting down.\n");
        exit(1);
    }
    else {
        printf("Connected to server on port %d.\n", port);
    }

    process_package(tcpSocket);

    close(tcpSocket);
}