#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "util.h"
#include "sigHandler.h"
#include "preparecmd.h"
#include "exec.h"

#define MAX_CMD_BUFFER 255
extern int last_status;
extern pid_t foreground_pid;
extern pid_t background_pid;

void redirect(char buffer[], char prevBuffer[], char *argv[]) {
	int status;
	int pid;
	char *prog_argv[MAX_CMD_BUFFER];
	int in = -1, out = -1;

	buildArgv(buffer, prog_argv);

	// Prepare command args and check for redirection
	char *cmd_args[MAX_CMD_BUFFER];
	int j = 0;

	for (int i = 0; prog_argv[i] != NULL; i++) {
		if (strcmp(prog_argv[i], "<") == 0 && prog_argv[i+1] != NULL) {
			in = open(prog_argv[++i], O_RDONLY);
			if (in < 0) {
				perror("Input file open failed");
				last_status = 1;
				return;
			}
		}
		else if (strcmp(prog_argv[i], ">") == 0 && prog_argv[i+1] != NULL) {
			out = open(prog_argv[++i], O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (out < 0) {
				perror("Output file open failed");
				last_status = 1;
				return;
			}
		} else {
			cmd_args[j++] = prog_argv[i];
		}
	}
	cmd_args[j] = NULL;

	pid = fork();
	if (pid < 0) {
		printf("fork failed\n");
		return;
	} 
	else if (!pid) {
		if (in != -1) {
			dup2(in, STDIN_FILENO);
			close(in);
		}
		if (out != -1) {
			dup2(out, STDOUT_FILENO);
			close(out);
		}
		
		execvp(cmd_args[0], cmd_args);
		perror("exec failed");
		exit(127);
	} 
	else {
		foreground_pid = pid;
		waitpid(pid, &status, WUNTRACED);
		foreground_pid = 0;

		if (WIFEXITED(status)) {
			last_status = WEXITSTATUS(status);
		}
		else if (WIFSIGNALED(status)) {
			last_status = 128 + WTERMSIG(status);
		}
	}
}

void runWithFile(char* argv[]) {
	printf("*******Script Mode*******\n");
	char buffer[MAX_CMD_BUFFER];
	char prevBuffer[MAX_CMD_BUFFER] = {0};
	FILE *fptr = fopen(argv[1], "r");
	if (fptr != NULL) {
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
}