#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include "preparecmd.h"
#include "sigHandler.h"
#include "util.h"
#include "execscript.h"

#define MAX_CMD_BUFFER 255

int last_status;
extern pid_t foreground_pid;

void performCmd(char buffer[], char cmd[], int validCode, char prevBuffer[], char *argv[]) {
	if (validCode == 1) {
		int idx = 0;
		for (int j = 0; buffer[j] != ' '; j++) {
			idx++;
		}
		idx++;

		if (strcmp(cmd, "echo") == 0) {
			if (buffer[idx] == '$' && buffer[idx+1] == '?') {
				printf("%d\n", last_status);
				return;
			}
			printStuff(buffer, idx);
			strcpy(prevBuffer, buffer);
		}

		else if (strcmp(cmd, "!!") == 0) {
			printf("%s", prevBuffer);
			execvp(prevBuffer, argv);
		}

		else if (strcmp(cmd, "exit") == 0) {
			exitHandler(buffer);
		}
		last_status = 0;
	}

	else {
		int status;
		int pid;
		char *prog_argv[MAX_CMD_BUFFER];

		buildArgv(buffer, prog_argv);
		pid = fork();
		if  (pid < 0) {
			perror("Fork failed\n");
			exit(errno);
			return;
		}
		else if (!pid) {
			execvp(prog_argv[0], prog_argv);
			
			perror("Bad command\n");
			exit(errno);
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
}

void runWithOutFile(char* argv[]) {
	char prevBuffer[MAX_CMD_BUFFER];
	char buffer[MAX_CMD_BUFFER]; // store the user command

	struct sigaction sig_int_action, sig_tstp_action;

	sigemptyset(&sig_int_action.sa_mask);
	sig_int_action.sa_handler = &handle_sigint;
	sig_int_action.sa_flags = SA_RESTART;

	sigemptyset(&sig_tstp_action.sa_mask);
	sig_tstp_action.sa_handler = &handle_sigtstp;
	sig_tstp_action.sa_flags = SA_RESTART;

	sigaction(SIGINT, &sig_int_action, NULL);
	sigaction(SIGTSTP, &sig_tstp_action, NULL);

	while (1) {
	    printf("icsh $ ");
		fflush(stdout);
		fgets(buffer, MAX_CMD_BUFFER, stdin);
		// check if redirection is present
		for (int i = 0; buffer[i] != '\0'; i++) {
			if (buffer[i] == '<' || buffer[i] == '>') {
				redirect(buffer, prevBuffer, argv);
				break;
			}
			else if (buffer[i+1] == '\0') {
				char cmd[MAX_CMD_BUFFER];
				parseCmd(buffer, cmd);
				performCmd(buffer, cmd, genValidCode(cmd), prevBuffer, argv);
				strcpy(prevBuffer, buffer);
			}
		}
	}
}