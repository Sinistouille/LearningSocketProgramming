#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "../packet/packet.h"

volatile sig_atomic_t stop;

int sendHttpResponse(SOCKET socket, const char *html);

BOOL WINAPI signalHandler(DWORD signal);

int main() {
    if (!SetConsoleCtrlHandler(signalHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }

    //SERVEUR
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    printf("Starting serveur Pid : %d\n", getpid());
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error creating socket : %d \n", WSAGetLastError());;
        return 0;
    }
    printf("Socket %llu\n", serverSocket);

    char ip[] = "192.168.1.143";
    int port = 5000;

    struct sockaddr_in servar_addr;
    servar_addr.sin_addr.s_addr = inet_addr(ip);
    servar_addr.sin_family = AF_INET;
    servar_addr.sin_port = htons(5000);

    if (bind(serverSocket, (struct sockaddr *) &servar_addr, sizeof(servar_addr)) == SOCKET_ERROR) {
        printf("Erreur connection : %c \n", WSAGetLastError());
        return -1;
    }
    printf("Socket bind to the adress %s:%d\n", ip, port);

    const char html[] =
        "<!DOCTYPE html>"
        "<html>"
        "<body>"
        "<h1>Hello world</h1>"
        "</body>"
        "</html>";

    while(!stop) {
        listen(serverSocket, 5);
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("Erreur lors de l'acceptation : %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
            break;
        }
        printf("\n\nSocket connecte %llu!\n\n", clientSocket);

        int bytes = 0;
        char *httpRequest = (char *) calloc(4,1);
        if (!httpRequest) {
            printf("Memory allocation failed\n");
            continue;
        }
        while (1) {
            char *buff = (char *) calloc(10, 1);
            memset(buff,0 ,10);

            const int bytesReceived = recv(clientSocket, buff, 9, 0);

            bytes += bytesReceived;
            printf("Received %d bytes\n", bytesReceived);
            if (bytesReceived > 0) {
                bytes += bytesReceived;
                httpRequest = (char *) realloc(httpRequest, bytes);

                strncat(httpRequest, buff, bytesReceived);
                if (strstr(httpRequest, "\r\n\r\n") != NULL) {
                    printf("\nEnd of HTTP request\n");
                    free(buff);
                    break;
                }
            }
            else if (bytesReceived == 0) {
                printf("Connection closed by client.\n");
                free(buff);
                break;
            } else {
                printf("Erreur lors de la reception : %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
                free(buff);
                break;
            }
            free(buff);
        }
        printf("%s", httpRequest);
        free(httpRequest);
        if (sendHttpResponse(clientSocket, html) == SOCKET_ERROR) {
            printf("Error sending HTTP response: %d\n", WSAGetLastError());
            break;
        }

        // Close client socket
        closesocket(clientSocket);
        printf("Client disconnected.\n");
    }

    shutdown(serverSocket, 0);

    closesocket(serverSocket);
    WSACleanup();
    printf("Finishing & closing server \n");
    return 0;
}

int sendHttpResponse(SOCKET socket, const char *html) {

    int headerSize = snprintf(NULL, 0,"HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %d \r\n"
    "\r\n", strlen(html));

    int httpResponseSize = headerSize + strlen(html) + 1;

    char *httpResponse = (char *) calloc(1, httpResponseSize);

    snprintf(httpResponse, httpResponseSize,"HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %d \r\n"
    "\r\n"
    "%s", strlen(html), html);

    int sendResult = send(socket, httpResponse, httpResponseSize, 0);

    printf("Sent HTTP response: \n%s\n", "");//httpResponse);
    free(httpResponse);

    return sendResult;
}

BOOL WINAPI signalHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT)
        stop = 1;
    printf("Signal intercepted\n");
    return TRUE;
}