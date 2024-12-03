#include <math.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t stop;

int sendHttpResponse(SOCKET socket, FILE *logs);

BOOL WINAPI signalHandler(DWORD signal);

int server(FILE *logs, char *ip, int port) {
    if (!SetConsoleCtrlHandler(signalHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }


    //SERVEUR
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error creating socket : %d \n", WSAGetLastError());
        return 0;
    }

    struct sockaddr_in servar_addr;
    servar_addr.sin_addr.s_addr = inet_addr(ip);
    servar_addr.sin_family = AF_INET;
    servar_addr.sin_port = htons(port);
    if (bind(serverSocket, (struct sockaddr *) &servar_addr, sizeof(servar_addr)) == SOCKET_ERROR) {
        printf("Erreur connection : %d \n", WSAGetLastError());
        return -1;
    }
    printf("Socket %llu bind to the adress %s:%d\n",serverSocket, ip, port);
    fprintf(logs,"Socket %llu bind to the adress %s:%d\n",serverSocket, ip, port);

    listen(serverSocket, 10);
    fprintf(logs, "Starting listening on socket : %llu", serverSocket);

    fd_set readfds;
    struct timeval timeout;

    while (!stop) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);

        timeout.tv_sec = 0;  // Wait 1 second
        timeout.tv_usec = 50000;

        // Use select to wait for a connection or timeout
        int activity = select(0, &readfds, NULL, NULL, &timeout);

        if (activity == SOCKET_ERROR) {
            printf("Select error: %d\n", WSAGetLastError());
            break;
        }

        if (activity == 0) {
            // Timeout: check for shutdown signal
            continue;
        }
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("Erreur lors de l'acceptation : %d\n", WSAGetLastError());  // For Windows; use `errno` on Linux.
            break;
        }
        struct sockaddr clientAddr;
        int len;
        getpeername(clientSocket, &clientAddr, &len);
        printf("\n\nSocket client %llu connecte %s!,\n\n", clientSocket, inet_ntoa(((struct sockaddr_in *) &clientAddr)->sin_addr));
        fprintf(logs,"\n\nSocket client %llu connecte %s!,\n\n", clientSocket, inet_ntoa(((struct sockaddr_in *) &clientAddr)->sin_addr));
        int bytes = 0;
        char *httpRequest = (char *) calloc(4,1);
        if (!httpRequest) {
            printf("Memory allocation failed\n");
            continue;
        }
        time_t t;
        time_t t1;
        time_t t2;
        time(&t1);
        time(&t);
        while (1) {
            FD_ZERO(&readfds);
            FD_SET(serverSocket, &readfds);

            timeout.tv_sec = 0;  // Wait 1 second
            timeout.tv_usec = 50000;

            // Use select to wait for a connection or timeout
            activity = select(0, &readfds, NULL, NULL, &timeout);

            if (activity == SOCKET_ERROR) {
                printf("Select error: %d\n", WSAGetLastError());
                break;
            }

            if (activity == 0) {
                //printf("No Activity detected while connected with socket %llu\n", clientSocket);
                time(&t2);
                int maxtime = 5;
                if(difftime(t2, t1) > maxtime) {
                    printf("Difftime : %f\n", difftime(t2, t1));
                    fprintf(logs,"Breaked the connection with socket %llu due to no response for (%ds)\n", clientSocket, maxtime);
                    break;
                }
                // Timeout: check for shutdown signal
            }
            if(difftime(t2,t) > 10) {
                printf("Difftime : %f\n", difftime(t2, t));
                fprintf(logs, "Breaked the connection with socket %llu due to max. time elapsed %ds\n", clientSocket, 10);
            }
            int size = 100;
            char *buff = (char *) calloc(size + 1, 1);
            memset(buff,0 ,size);

            const int bytesReceived = recv(clientSocket, buff, size, 0);

            printf("Received %d bytes\n", bytesReceived);
            if (bytesReceived > 0) {
                time(&t1);
                bytes += bytesReceived;
                httpRequest = (char *) realloc(httpRequest, bytes);

                strncat(httpRequest, buff, bytesReceived);
                if (strstr(httpRequest, "\r\n\r\n") != NULL) {
                    printf("%s\n", httpRequest);
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
        //printf("Received %d Bytes\n", bytes);
        fprintf(logs,"Receiveid %d bytes from %llu\nRequest :\n%s", bytes, clientSocket, httpRequest);
        free(httpRequest);
        if (sendHttpResponse(clientSocket, logs) == SOCKET_ERROR) {
            printf("Error sending HTTP response: %d\n", WSAGetLastError());
            break;
        }

        // Close client socket
        closesocket(clientSocket);
        printf("Client disconnected.\n");
        fprintf(logs,"Client %llu disconnected.\n", clientSocket);
    }
    shutdown(serverSocket, 0);

    closesocket(serverSocket);
    WSACleanup();
    printf("Finishing & closing server \n");
    fprintf(logs,"Shutdown server : closing his socket %llu\n", serverSocket);
    fclose(logs);
    return 0;
}

int sendHttpResponse(SOCKET socket, FILE *logs) {

    FILE *fHtml = fopen("./pages/index.html","r");
    if(fHtml == NULL) {
        printf("Error opening file\n");
        return -1;
    }
    char *html = calloc(1024, 1);
    char line[50];
    int i = 0;
    while(fgets(line,50, fHtml)) {
        //printf("File : %s", line);
        i++;
        if(i*50 > strlen(html)) {
            html = realloc(html, i*50);
        }
        strcat(html, line);
    }
    fprintf(logs,"HTML File :\n %s",html);

    char *header = "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: %d \r\n"
    "\r\n"
    "%s";

    int httpResponseSize = strlen(header) + (int) log10(strlen(html)) + strlen(html) - 2;
    char *httpResponse = (char *) calloc(1, httpResponseSize);
    snprintf(httpResponse, httpResponseSize,header, strlen(html), html);

    int sendResult = send(socket, httpResponse, httpResponseSize, 0);

    //printf("Sent HTTP response: \n%s\n", httpResponse);
    fprintf(logs,"Sent HTTP response: \n%s\n\n", httpResponse);
    free(httpResponse);
    fclose(fHtml);
    free(html);
    return sendResult;
}

BOOL WINAPI signalHandler(DWORD signal) {
    printf("Signal intercepted\n");
    if (signal == CTRL_C_EVENT) {
        stop = 1;
        printf("Shuting down server : Ctrl+C action\n");
    }
    return TRUE;
}