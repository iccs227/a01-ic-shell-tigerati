#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_BUFFER 255

void parseCmd(char *buffer, char *cmd) {
	int i = 0;
	while (buffer[i] != ' ' && buffer[i] != '\n' && buffer[i] != '\0') {
		cmd[i] = buffer[i];
		i++;
	}
	cmd[i] = '\0';
}

void buildArgv(char buffer[], char *argv[]) {
	int i = 0;
	char *token = strtok(buffer, " \n");
	while (token != NULL && i < MAX_CMD_BUFFER-1) {
		argv[i++] = token;
		token = strtok(NULL, " \n");
	}
	argv[i] = NULL;
}