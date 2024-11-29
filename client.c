//
// Created by Maxence on 02/10/2024.
//
#include <stdint.h>
#include <stdio.h>
#include <winsock2.h>
#include <unistd.h>
#include "src/packet.h"


int main()
{
    //CLIENT
    // creating socket
    WSADATA data;
    WSAStartup(MAKEWORD(2,2), &data);
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // specifying address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6666);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // sending connection request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress,
            sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("Connection failed\n");
        return 1;
    }

    // sending data
    for(short i = 0; i < 20; i++) {
        PACKET packet;
        packet.type = TYPE_CHAR;// Increase buffer size to hold the final message
        snprintf(packet.message, sizeof(packet.message), "Message : %d \0", i);

        int bytesSent = send(clientSocket, (char*) &packet, sizeof(packet), 0);
        if (bytesSent == SOCKET_ERROR) {
            printf("Erreur lors de l'envoi: %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
        } else {
            printf("Message sent: %s, type : %d\n", packet.message, packet.type);
        }
    }
    PACKET packet;
    packet.type = TYPE_INT32;
    snprintf(packet.message, sizeof(packet.message),"%d",42);
    send(clientSocket, (char *) &packet, sizeof(packet), 0);

    packet.type = TYPE_FLOAT;
    snprintf(packet.message, sizeof(packet.message),"%f",42.55);
    send(clientSocket, (char *) &packet, sizeof(packet), 0);

    packet.type = TYPE_CHAR;
    snprintf(packet.message, sizeof(packet.message),"close\0");
    send(clientSocket, (char *) &packet, sizeof(packet), 0);

    //sleep(5);

    // closing socket
    shutdown(clientSocket,0);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}