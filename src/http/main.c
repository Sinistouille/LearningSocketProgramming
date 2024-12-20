//
// Created by Maxence on 03/12/2024.
//
#include <io.h>
#include <process.h>
#include <time.h>
#include <winsock2.h>

#include "./server/httpserver.h"

int getipfromConf(char *ip, int *port);

int start(FILE **logs);

int main() {

	FILE *logs;
	if(start(&logs) == -1 || logs == NULL) {
		return -1;
	}
	// printf("main\n");
	// printf("Adresse &logs: %p\n", &logs);
	// printf("Adresse logs : %p\n", logs);

	//dynamically allocating space for the IP
	char *ip = (char * ) calloc(19,1);
	if(ip == NULL) {
		printf("Cannot allocate memory for ip\n");
		fprintf(logs,"Error while allocating memory for ip\n");
		return -1;
	}
	int port = 0;

	//starting the loop with the cond to close the server
	int close = 0;
	while(!close){
		//reading the IP from the config file
		if(getipfromConf(ip,&port) == -1) {
			printf("Cannot open config file\n");
			fprintf(logs,"Error while opening config file \n");
			return -1;
		}

		//calling the function to start the socket
		if(server(&logs, ip, &port) == -1) {
			printf("Erreur connection : %d \n", WSAGetLastError());
			return -1;
		}

		//asking the user if he wants to close the server
		//dinamically allocating space for the input
		char* input = calloc(2,1);
		printf("Do you want to close the server ? Type (Y) for Yes, Anything else for No\n");
		if(fgets(input,2,stdin) == NULL) {
			printf("No input detected");
		}else {
			if(strstr(input,"Y") != NULL) {
				close = 1;
			}
			//clearing stdin from garbage
			while (getchar() != '\n' && !feof(stdin)){};
			free(input);
		}
	}
	printf("Closing processus\n");
	fclose(logs);
	free(ip);
}

int getipfromConf(char *ip, int *port) {
	FILE *conf = fopen("./config/conf.txt", "r");
	if(conf == NULL) {
		return -1;
	}


	char *line = (char *) calloc(25,1);
	fgets(line,25,conf);
	fclose(conf);

	const char *occ = strchr(line,':');
	if( occ == NULL) {
		printf("Error while trying to find : in conf file\n");
		return -1;
	}
	strncpy(ip,line,  occ -line);
	ip[occ - line] = '\0';



	char *portc = calloc(5,1);
	strncpy(portc,occ + 1, 5);
	*port = atoi(portc);

	//printf("String line %s \nString ip : %s, String port %s, ATOI port %d\n", line, ip, portc, *port);

	free(line);
	free(portc);
	return 0;
}

//procedure to before starting the server
//opening the logs file
//creating if needed the dir logs
//printing time
int start(FILE **logs) {
	mkdir("./logs");
	*logs = fopen("./logs/logs.txt", "w");
	if(*logs == NULL) {
		printf("Cannot open logs file\n");
		return -1;
	}
	// printf("start\n");
	// printf("Adresse &logs: %p\n", &logs);
	// printf("Adresse logs : %p\n", logs);
	// printf("Adresse *logs: %p\n", *logs);
	time_t t;
	time(&t);
	fprintf(*logs, "Starting server at %sPID : %d\n", ctime(&t),getpid());
	printf("Starting server at %sPID : %d\n", ctime(&t),getpid());
	return 0;
}