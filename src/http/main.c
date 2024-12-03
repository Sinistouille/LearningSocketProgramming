//
// Created by Maxence on 03/12/2024.
//
#include <io.h>
#include <process.h>
#include <time.h>
#include <winsock2.h>

#include "./server/httpserver.h"

int getipfromConf(char *ip, int *port);

int main() {

	mkdir("./logs");
	FILE *logs = fopen("./logs/logs.txt", "w");
	if(logs == NULL) {
		printf("Cannot open logs file\n");
		return -1;
	}

	time_t t;
	time(&t);
	fprintf(logs, "Starting server at %s, PID : %d\n", ctime(&t),getpid());
	printf("Starting server at %sPID : %d\n", ctime(&t),getpid());

	char *ip = (char * ) calloc(19,1);
	int port = 0;

	if(getipfromConf(ip,&port) == -1) {
		printf("Cannot open config file\n");
		fprintf(logs,"Error while opening config file \n");
		return -1;
	}
	printf("Outside : IP : %s, Port : %d\n", ip, port);



	server(logs, ip, port);

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

	printf("String line %s \nString ip : %s, String port %s, ATOI port %d\n", line, ip, portc, *port);

	free(line);
	free(portc);
	return 0;
}