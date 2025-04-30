/* ICCS227: Project 1: icsh
 * Name: Athiwat Panboonprung
 * StudentID: 6680729
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "unistd.h"
#include "sys/wait.h"
#include "signal.h"

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

void buildArgv(char buffer[], char *argv[]) {
	int i = 0;
	char *token = strtok(buffer, " \n");
	while (token != '\0' && i < MAX_CMD_BUFFER-1) {
		argv[i++] = token;
		token = strtok('\0', " \n");
	}
	argv[i] = '\0';
}

void exitHandler(char buffer[]) {
	// Skip the space
	int idx = 0;
	for (int j = 0; buffer[j] != ' '; j++) {
		idx++;
	}
	idx++;
	
	char num[MAX_CMD_BUFFER];
	for (int j = 0; buffer[idx] != '\0'; j++) {
		num[j] = buffer[idx++];
	}
	int exitCode = atoi(num);

	if (exitCode > 255) {
		exit(exitCode & 0xFF);
	}
	exit(exitCode);
}

void handle_sigtstp(int sig) {
	
}

void performCmd(char buffer[], char cmd[], int validCode, char prevBuffer[], char *argv[]) {
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
			printf("%s", prevBuffer);
			printStuff(prevBuffer, idx);
		}

		else if (strcmp(cmd, "exit") == 0) {
			exitHandler(buffer);
		}
	}

	else {
		int status;
		int pid;
		char *prog_argv[MAX_CMD_BUFFER];

		buildArgv(buffer, prog_argv);
		pid = fork();
		if  (pid < 0) {
			printf("fork failed\n");
			exitHandler(buffer);
		}
		else if (!pid) {
			execvp(prog_argv[0], prog_argv);
		}
		else {
			waitpid(pid, NULL, 0);
		}
	}
}

void runWithOutFile(char* argv[]) {
	char prevBuffer[MAX_CMD_BUFFER];
	char buffer[MAX_CMD_BUFFER];

	while (1) {
	    printf("icsh $ ");
		fgets(buffer, MAX_CMD_BUFFER, stdin);
		char cmd[MAX_CMD_BUFFER];
		parseCmd(buffer, cmd);
		performCmd(buffer, cmd, genValidCode(cmd), prevBuffer, argv);
	}
}

void runWithFile(char* argv[]) {
	char buffer[MAX_CMD_BUFFER];
	FILE *fptr = fopen(argv[1], "r");
	char prevBuffer[MAX_CMD_BUFFER] = {0};
	while (fgets(buffer, MAX_CMD_BUFFER, fptr)) {
		char cmd[MAX_CMD_BUFFER];
		parseCmd(buffer, cmd);
		performCmd(buffer, cmd, genValidCode(cmd), prevBuffer, argv);
		if (strcmp(cmd, "!!") != 0) {
			strcpy(prevBuffer, buffer);
		}
	}
	fclose(fptr);
}

int main(int argc, char* argv[]) {
	runWithOutFile(argv);
}
