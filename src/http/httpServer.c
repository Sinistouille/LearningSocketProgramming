#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t stop;

int sendHttpResponse(SOCKET socket, const char *html, FILE *logs);

BOOL WINAPI signalHandler(DWORD signal);

int main() {
    if (!SetConsoleCtrlHandler(signalHandler, TRUE)) {
        printf("\nERROR: Could not set control handler");
        return 1;
    }

    FILE *logs = fopen("./logs/logs.txt", "w");
    if(logs == NULL) {
        printf("Cannot open logs file\n");
        return -1;
    }
    time_t t;
    time(&t);
    fprintf(logs, "Starting server at %s\n", ctime(&t));
    //SERVEUR
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    printf("Starting serveur Pid : %d\n", getpid());
    fprintf(logs,"Starting serveur Pid : %d\n", getpid());
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("Error creating socket : %d \n", WSAGetLastError());;
        return 0;
    }
    char ip[] = "172.16.5.33";
    int port = 5000;

    struct sockaddr_in servar_addr;
    servar_addr.sin_addr.s_addr = inet_addr(ip);
    servar_addr.sin_family = AF_INET;
    servar_addr.sin_port = htons(5000);

    if (bind(serverSocket, (struct sockaddr *) &servar_addr, sizeof(servar_addr)) == SOCKET_ERROR) {
        printf("Erreur connection : %c \n", WSAGetLastError());
        return -1;
    }
    printf("Socket %llu bind to the adress %s:%d\n",serverSocket, ip, port);
    fprintf(logs,"Socket %llu bind to the adress %s:%d\n",serverSocket, ip, port);
    FILE *fHtml = fopen("./pages/index.html","r");
    if(fHtml == NULL) {
        printf("Error opening file\n");
        return -1;
    }

    char *html = calloc(1024, 1);
    char line[50];
    while(fgets(line,50, fHtml)) {
        //printf("File : %s", line);
        strcat(html, line);
    }
    fprintf(logs,"HTML File :\n %s",html);

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
        printf("\n\nSocket connecte %llu!\n\n", clientSocket);
        fprintf(logs,"\n\nSocket connecte %llu!\n\n", clientSocket);
        int bytes = 0;
        char *httpRequest = (char *) calloc(4,1);
        if (!httpRequest) {
            printf("Memory allocation failed\n");
            continue;
        }
        while (1) {
            char *buff = (char *) calloc(10, 1);
            memset(buff,0 ,9);

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
        printf("Received %d Bytes\n", bytes);
        fprintf(logs,"Receiveid %d bytes from %llu\nRequest :\n%s", bytes, clientSocket, httpRequest);
        printf("%s", httpRequest);
        free(httpRequest);
        if (sendHttpResponse(clientSocket, html, logs) == SOCKET_ERROR) {
            printf("Error sending HTTP response: %d\n", WSAGetLastError());
            break;
        }

        // Close client socket
        closesocket(clientSocket);
        printf("Client disconnected.\n");
        fprintf(logs,"Client %llu disconnected.\n", clientSocket);
    }
    fclose(fHtml);
    free(html);
    shutdown(serverSocket, 0);

    closesocket(serverSocket);
    WSACleanup();
    printf("Finishing & closing server \n");
    fprintf(logs,"Shutdown server : closing his socket %llu\n", serverSocket);
    fclose(logs);
    return 0;
}

int sendHttpResponse(SOCKET socket, const char *html, FILE *logs) {

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

    //printf("Sent HTTP response: \n%s\n", httpResponse);
    fprintf(logs,"Sent HTTP response: \n%s\n\n", httpResponse);
    free(httpResponse);

    return sendResult;
}

BOOL WINAPI signalHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT)
        stop = 1;
    printf("Signal intercepted\n");
    return TRUE;
}