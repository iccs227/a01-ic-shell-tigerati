#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CMD_BUFFER 255

int genValidCode(char cmd[]) {
	char cmds[7][5] = {"echo", "!!", "exit", "jobs", "fg", "bg", "cd"};
	int validCode = 0;
	for (int j = 0; j < 7; j++) {
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

int extractJobId(char buffer[]) {
	char *ptr = strchr(buffer, '%');
	int idx = ptr - buffer+1;
	
	char num[MAX_CMD_BUFFER];
	for (int j = 0; buffer[idx] >= '0' && buffer[idx] <= '9'; j++) {
		num[j] = buffer[idx++];
	}
	num[idx] = '\0';
	return atoi(num);
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