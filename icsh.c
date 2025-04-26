/* ICCS227: Project 1: icsh
 * Name: Athiwat Panboonprung
 * StudentID: 6680729
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"

#define MAX_CMD_BUFFER 255

void parseCmd(char *buffer, char *cmd) {
	int i = 0;
	while (buffer[i] != ' ' && buffer[i] != '\n' && buffer[i] != '\0') {
		cmd[i] = buffer[i];
		i++;
	}
	cmd[i] = '\0';
}

int genValidCode(char cmd[]) {
	char cmds[3][MAX_CMD_BUFFER] = {"echo", "!!", "exit"};	
	int validCode = 0;
	for (int j = 0; j < 3; j++) {
		if (strcmp(cmds[j], cmd) == 0) {
			validCode = 1;
		}
	}
	return validCode;
}

void printStuff(char buffer[], int idx) {
	while (buffer[idx] != '\0') {
		printf("%c", buffer[idx]);
		idx++;
	}
}

void performCmd(char buffer[], char cmd[], int validCode, char prevBuffer[]) {
	if (validCode == 1) {
		int idx = 0;
		for (int j = 0; buffer[j] != ' '; j++) {
			idx++;
		}
		idx++;
		if (strcmp(cmd, "echo") == 0) {
			printStuff(buffer, idx);
			strcpy(prevBuffer, buffer);
		}
		else if (strcmp(cmd, "!!") == 0) {
			printStuff(prevBuffer, idx);
		}
		else {
			printf("bye\n");
			char num[MAX_CMD_BUFFER];
			for (int j = 0; buffer[idx] != '\0'; j++) {
				num[j] = buffer[idx];
				idx++;
			}
			int exitCode = atoi(num);
			if (exitCode > 255) {
				exit(exitCode & 0xFF);
			}
			exit(exitCode);
		}
	}
	else if (buffer[0] != '\n' && validCode == 0) {
		printf("bad command\n");
	}
}

void runWithOutFile() {
	char prevBuffer[MAX_CMD_BUFFER];
	char buffer[MAX_CMD_BUFFER]; 
	while (1) {
	        printf("icsh $ ");
		fgets(buffer, MAX_CMD_BUFFER, stdin);
		char cmd[MAX_CMD_BUFFER];
		parseCmd(buffer, cmd);
		performCmd(buffer, cmd, genValidCode(cmd), prevBuffer);
	}
}

int main(int argc, char* argv[]) {
    char buffer[MAX_CMD_BUFFER];
    if (argc == 1) {
	runWithOutFile();
    }
    else {
	FILE *fptr = fopen(argv[1], "r");
	char prevBuffer[MAX_CMD_BUFFER] = {0};
	while (fgets(buffer, MAX_CMD_BUFFER, fptr)) {
		char cmd[MAX_CMD_BUFFER];
		parseCmd(buffer, cmd);
		performCmd(buffer, cmd, genValidCode(cmd), prevBuffer);
		if (strcmp(cmd, "!!") != 0) {
			strcpy(prevBuffer, buffer);
		}
	}
	fclose(fptr);
    }
}
