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
#include "background.h"
#include "job.h"

#define MAX_CMD_BUFFER 255

Job jobs[MAX_CMD_BUFFER];
char ori_buffer[MAX_CMD_BUFFER];
int job_count = 0;
int last_status;
pid_t foreground_pid;
extern pid_t background_pid;

void runBuildInCmd(char buffer[], char cmd[], char prevBuffer[], char *argv[]) {
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
	}

	else if (strcmp(cmd, "fg") == 0) {
		backToForeground(extractJobId(ori_buffer));
	}

	else if (strcmp(cmd, "bg") == 0) {
		int idx = extractJobId(ori_buffer) - 1;

		if (idx + 1 < 1 || idx + 1 > job_count) {
			printf("No such job\n");
			return;
		}

		pid_t pid = jobs[idx].pid;
		if (pid == foreground_pid) {
			printf("Job %d is already in the foreground\n", idx+1);
			return;
		}
		
		strcpy(jobs[idx].status, "Running");
		kill(pid, SIGCONT);
		printf("[%d]%c %s", jobs[idx].job_id, jobs[idx].indicator, jobs[idx].command);
		
	}

	else if (strcmp(cmd, "cd") == 0) {
		char *path = buffer + idx;
		path[strcspn(path, "\n")] = '\0'; // Remove newline character
		if (chdir(path) != 0) {
			perror("cd failed");
			last_status = errno;
		} else {
			last_status = 0;
		}
	}

	else if (strcmp(cmd, "exit") == 0) {
		exitHandler(buffer);
	}

	else if (strcmp(cmd, "jobs") == 0) {
		jobCmd();
	}

	else if (strcmp(cmd, "!!") == 0) {
		pid_t pid = fork();
		if (pid == 0) {
			char cmd[MAX_CMD_BUFFER];
			parseCmd(prevBuffer, cmd);

			printf("%s", prevBuffer);
			char *prog_argv[MAX_CMD_BUFFER];
			buildArgv(prevBuffer, prog_argv);
			execvp(prog_argv[0], prog_argv);

			perror("Bad command");
			fflush(stdout);
			exit(1);
			
		} else {
			int status;
			waitpid(pid, &status, 0);
			last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
		}
		return;
	}
	last_status = 0;
}

void performCmd(char buffer[], char cmd[], char prevBuffer[], char *argv[]) {
	int status;
	int pid;
		
	pid = fork();
	if  (pid < 0) {
		perror("Fork failed\n");
		exit(errno);
		return;
	}
	else if (!pid) {
		// Child process
		char *prog_argv[MAX_CMD_BUFFER];
		buildArgv(buffer, prog_argv);
		execvp(prog_argv[0], prog_argv);
		
		perror("Bad command");
		fflush(stdout);
		exit(errno);
	}
	else {
		foreground_pid = pid;
		waitpid(pid, &status, WUNTRACED);
		
		if (WIFEXITED(status)) {
			last_status = WEXITSTATUS(status);
		}
		if (WIFSTOPPED(status)) {
			// Add job to the background
			printf("\n");
			addJob(pid, ori_buffer, "Stopped", '+');
			printf("[%d]%c %s\t%s", job_count, jobs[job_count - 1].indicator, jobs[job_count - 1].status, jobs[job_count - 1].command);
			foreground_pid = 0; // Reset foreground process
		}
		else if (WIFSIGNALED(status)) {
			printf("Process terminated by signal %d\n", WTERMSIG(status));
			last_status = status;
		}
		else {
			last_status = 1;
		}
	}
}

void runWithOutFile(char* argv[]) {
	char prevBuffer[MAX_CMD_BUFFER];
	char buffer[MAX_CMD_BUFFER]; // store the user command

	struct sigaction sig_int_action, sig_tstp_action, sig_cont_action, sig_chld_action;

	sigemptyset(&sig_chld_action.sa_mask);
	sig_chld_action.sa_handler = &handle_sigchld;
	sig_chld_action.sa_flags = SA_RESTART;

	sigemptyset(&sig_int_action.sa_mask);
	sig_int_action.sa_handler = &handle_sigint;
	sig_int_action.sa_flags = SA_RESTART;

	sigemptyset(&sig_tstp_action.sa_mask);
	sig_tstp_action.sa_handler = &handle_sigtstp;
	sig_tstp_action.sa_flags = SA_RESTART;

	sigemptyset(&sig_cont_action.sa_mask);
	sig_cont_action.sa_handler = &handle_sigcont;
	sig_cont_action.sa_flags = SA_RESTART;
	
	sigaction(SIGCONT, &sig_cont_action, NULL);
	sigaction(SIGINT, &sig_int_action, NULL);
	sigaction(SIGTSTP, &sig_tstp_action, NULL);
	sigaction(SIGCHLD, &sig_chld_action, NULL);

	while (1) {
	    printf("icsh $ ");
		fflush(stdout);
		fgets(buffer, MAX_CMD_BUFFER, stdin);
		strcpy(ori_buffer, buffer);
		// check if redirection is present
		if (strchr(buffer, '<') != NULL || strchr(buffer, '>') != NULL) {
			redirect(buffer, prevBuffer, argv);
			continue; // Skip further processing if redirection is handled
		}

		if (strchr(buffer, '&') != NULL) {
			buffer[strcspn(buffer, "&")] = '\0'; // Remove '&' from the command
			createBackgroundJob(buffer);
			continue; // Skip further processing if background job is created
		}

		char cmd[MAX_CMD_BUFFER];
		parseCmd(buffer, cmd);

		int validCode = genValidCode(cmd);

		if (validCode) { runBuildInCmd(buffer, cmd, prevBuffer, argv); }
		
		else { performCmd(buffer, cmd, prevBuffer, argv); }
		
		memset(prevBuffer, '\0', MAX_CMD_BUFFER);
		strcpy(prevBuffer, ori_buffer);
	}
}