#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

pid_t foreground_pid;
pid_t background_pid;

void handle_sigtstp(int sig) {
	if (foreground_pid == 0) {
		printf("\nSigTSTP, no foreground process to stop\n");
		printf("icsh $ ");
		fflush(stdout);
		return;
	}

	background_pid = foreground_pid;
	kill(foreground_pid, SIGTSTP);
	printf("\n");
}

void handle_sigint(int sig) {
	if (foreground_pid == 0) {
		printf("\nSigINT, no foreground process to stop\n");
		printf("icsh $ ");
		fflush(stdout);
		return;
	}

	background_pid = foreground_pid;
	kill(foreground_pid, SIGINT);
	printf("\n");
}