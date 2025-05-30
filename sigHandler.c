#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "job.h"
#include "background.h"

extern pid_t foreground_pid;
extern Job jobs[];
extern int job_count;

void handle_sigchld(int sig) {
	int status;
	pid_t pid;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		for (int i = 0; i < job_count; i++) {
			if (jobs[i].pid == pid) {
				if (WIFEXITED(status)) {
					strcpy(jobs[i].status, "Done");
					printf("\n[%d] %s\t%s\n", jobs[i].job_id, jobs[i].status, jobs[i].command);
					removeJob(i);
				}
				else if (WIFSIGNALED(status)) {
					strcpy(jobs[i].status, "Terminated");
				}
			}
		}
	}
}

void handle_sigcont(int sig) {
	if (foreground_pid == 0) {
		printf("\nSigCONT, no foreground process\n");
		printf("icsh $ ");
		fflush(stdout);
		return;
	}
	printf("\n");
	kill(foreground_pid, sig);
}

void handle_sigtstp(int sig) {
	if (foreground_pid == 0) {
		printf("\nSigTSTP, no foreground process\n");
		printf("icsh $ ");
		fflush(stdout);
		return;
	}
	printf("\n");
	kill(foreground_pid, sig);
}

void handle_sigint(int sig) {
	if (foreground_pid == 0) {
		printf("\nSigINT, no foreground process to stop\n");
		printf("icsh $ ");
		fflush(stdout);
		return;
	}
	printf("\n");
	kill(foreground_pid, sig);
}