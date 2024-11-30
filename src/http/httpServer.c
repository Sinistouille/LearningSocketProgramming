#include <io.h>
#include <stdio.h>
#include <winsock2.h>
#include <time.h>
#include "../packet/packet.h"

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
    servar_addr.sin_port = htons(5000);
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
    char *c;
    while (1) {
        c = (char *) calloc(1, 1);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate elapsed time in nanoseconds
        const long seconds = end.tv_sec - start.tv_sec;
        const long ms = (end.tv_nsec - start.tv_nsec) / 1e6;
        const int bytesReceived = recv(clientSocket, c, sizeof(1), 0);
        if (bytesReceived > 0) {
            if(strcmp(c, "\r") == 0) {
                printf("\\r", c);
            }
            if(strcmp(c, "\n") == 0) {
                printf("\\n", c);
            }
            printf("%s", c);
            if(strcmp(c, "\r") == 0 || strcmp(c, "\n") == 0) {
                break;
            }
        } else if (bytesReceived == 0) {
            printf("Connection closed by client.\n");
            break;
        } else {
            printf("Erreur lors de la reception : %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
            break;
        }
        free(c);
    }
    free(c);
    const char html[] =
    "<!DOCTYPE html>"
    "<html>"
    "<body>"
    "<h1>Hello world</h1>"
    "</body>"
    "</html>";

    int headerSize = snprintf(NULL, 0,"HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %d \r\n"
    "\r\n", strlen(html));

    int httpResponseSize = headerSize + strlen(html) + 1;

    char *httpResponse  = calloc(httpResponseSize, 1);
    snprintf(httpResponse, httpResponseSize,"HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %d \r\n"
    "\r\n"
    "%s", strlen(html), html);

    send(clientSocket, httpResponse, httpResponseSize, 0);
    printf("Sent HTTP response: %s\n", httpResponse);

    free(httpResponse);
    shutdown(serverSocket, 0);
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}