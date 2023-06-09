#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#include "defines.h"


int g_live_threads = 0;
int g_keep_alive = 1;

void* process_package(void* tcpConnectionPtr)
{
    int tcpConnection = *(int*)tcpConnectionPtr;
    // Infinite loop for client - can exit with 'q' from client
    for (;;) {
        char message_buffer[RW_BUFFER_MAX];
        bzero(message_buffer, RW_BUFFER_MAX);

        // Receive
        int read_bytes = read(tcpConnection, message_buffer, sizeof(message_buffer));
        printf("Incoming message was: %s", message_buffer);

        // Reverse
        // 1. find line end -> purposefully go beyond
        int end = read_bytes;
        while(message_buffer[end--] != '\n');

        // 2. Check whether it's an exit
        if(end == 0 && strcmp(message_buffer, "q\n") == 0) {
            printf("Shutting down.\n");
            --g_live_threads;
            g_keep_alive = 0;
            break;
        }

        // 3. reverse in-place
        for(int front = 0; front < end; ++front, --end) {
            char tmp = message_buffer[front];
            message_buffer[front] = message_buffer[end];
            message_buffer[end] = tmp;
        }

        // Send
        printf("Outgoing message is: %s", message_buffer);
        write(tcpConnection, message_buffer, sizeof(message_buffer));
    }
    close(tcpConnection);
    pthread_exit(NULL);
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // The port, starting from default and trying a few times, should there be more
    // servers running on the host.
    int bind_result = -1;
    int port = DEFAULT_PORT;
    for(; bind_result != 0 && port < DEFAULT_PORT + 100; ++port) {
        servaddr.sin_port = htons(port);
        bind_result = bind(tcpSocket, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if(bind_result != 0) {
            printf("Failed to bind socket to port %d...\n", port);
        }
    }
    if (bind_result != 0) {
        printf("Cannot bind socket to port. Shutting down.\n");
        exit(1);
    }
    else {
        printf("Socket successfully bound to port %d.\n", port);
    }

    // Try to listen on the port for the client to connect.
    if ((listen(tcpSocket, MAX_CONNECTIONS)) != 0) {
        printf("TCP Socket listen failed. Server shutting down...\n");
        exit(1);
    }
    else {
        printf("Server listening on port %d...\n", port);
    }

    // Create a new thread for every incoming connection with a maximum number predefined.
    pthread_t threads[MAX_CONNECTIONS];
    struct sockaddr_storage serverStorage;
    int len = sizeof(serverStorage);
    int tcpConnection = -1;
    int currentThreadId = -1;
    do {
        if(g_live_threads == MAX_CONNECTIONS) {
            printf("Maximum connections reached; no longer accepting connections.\n");
            char c = getchar();
            continue;
        }
        if(!g_keep_alive) {
            printf("Shutdown signal received; no longer accepting connections.\n");
            char c = getchar();
            continue;
        }

        tcpConnection = accept(tcpSocket, (struct sockaddr*)&serverStorage, &len);
        if (tcpConnection < 0) {
            printf("Failed to accept client connection.\n");
            continue;
        }
        else {
            printf("Client connected.\n");
        }

        if(pthread_create(&threads[++currentThreadId], NULL, process_package, &tcpConnection) != 0) {
            printf("Failed to create thread [%d].\n", currentThreadId);
            continue;
        }
        ++g_live_threads;
        printf("Client thread [%d] created. Processing...\n", currentThreadId);
    } while(g_keep_alive);

    for(int i = 0; i <= currentThreadId; ++i) {
        pthread_join(threads[i], NULL);
        printf("Thread [%d] joined.\n", i);
    }

    close(tcpSocket);
}
