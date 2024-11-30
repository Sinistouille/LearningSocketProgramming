#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <time.h>
#include "packet/packet.h"

int main() {
    //SERVEUR
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error creating socket : %d \n", WSAGetLastError());;
        return 0;
    }
    printf("Socket %llu\n", serverSocket);
    struct sockaddr_in servar_addr;
    servar_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servar_addr.sin_family = AF_INET;
    servar_addr.sin_port = htons(6666);
    if (bind(serverSocket, (struct sockaddr *) &servar_addr, sizeof(servar_addr)) == SOCKET_ERROR) {
        printf("Erreur connection : %c \n", WSAGetLastError());
        return -1;
    }
    listen(serverSocket, 5);
    struct timespec start, end;
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        printf("Erreur lors de l'acceptation : %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
        return -1;
    }
    printf("Socket connecte !\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (1) {
        PACKET packet;
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate elapsed time in nanoseconds
        const long seconds = end.tv_sec - start.tv_sec;
        const long ms = (end.tv_nsec - start.tv_nsec) / 1e6;
        printf("Elapsed time: %d s, %d ms\n", seconds, ms);
        const int bytesReceived = recv(clientSocket, (char*) &packet, sizeof(packet), 0);
        if (bytesReceived > 0) {
            if(packet.type == TYPE_INT32)
                printf("Message from socket int %llu: %d\n", clientSocket,atoi(packet.message));
            if(packet.type == TYPE_FLOAT)
                printf("Message from socket float %llu: %f\n", clientSocket,atof(packet.message));
            if(packet.type == TYPE_CHAR)
                printf("Message from socket char %llu: %s\n", clientSocket,packet.message);
            else {
                printf("Message from socket char %llu: %s\n", clientSocket,packet.message);
            }
            if(strcmp(packet.message, "close") == 0) {
                break;
            }
        } else if (bytesReceived == 0) {
            printf("Connection closed by client.\n");
            break;
        } else {
            printf("Erreur lors de la reception : %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
            break;
        }
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}