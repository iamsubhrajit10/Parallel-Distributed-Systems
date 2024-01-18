#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 4040

void *handleClient(void *arg) {
    int newSocket = *((int *)arg);
    char buffer[1024];
    bzero(buffer, sizeof(buffer));

    while (1) {
        ssize_t recvStatus = recv(newSocket, buffer, sizeof(buffer), 0);

        if (recvStatus <= 0) {
            // Either an error or the client disconnected
            if (recvStatus == 0) {
                printf("Client disconnected.\n");
            } else {
                printf("Error in receiving data");
            }
            break;
        }

        if (strcmp(buffer, ":exit") == 0) {
            printf("Client disconnected.\n");
            break;
        } else {
            printf("Client: %s\n", buffer);
            send(newSocket, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof(buffer));
        }
    }

    // Close the socket and remove the thread
    close(newSocket);
    pthread_exit(NULL);
}



int main() {
    int sockfd, ret;
    struct sockaddr_in serverAddr;
    int newSocket;
    struct sockaddr_in newAddr;
    socklen_t addr_size;
    pthread_t tid[50];  // Assuming a maximum of 50 clients
    int clientCount=0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    ret = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0) {
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", PORT);

    if (listen(sockfd, 10) == 0) {
        printf("[+]Listening....\n");
    } else {
        printf("[-]Error in binding.\n");
    }
    
    while (1) {
        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        if (newSocket < 0) {
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        // Create a new thread to handle the client
        if (pthread_create(&tid[clientCount], NULL, handleClient, &newSocket) != 0) {
            printf("Failed to create thread.\n");
        }
        sleep(1);
        clientCount++;
    }

    close(sockfd);

    return 0;
}
