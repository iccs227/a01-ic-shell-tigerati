#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include "job.h"
#include "preparecmd.h"
#include "util.h"

#define MAX_CMD_BUFFER 255

extern int last_status;
extern pid_t foreground_pid;
extern pid_t background_pid;
extern int job_count;
extern Job jobs[];
extern char ori_buffer[];

void addJob(int pid, char command[], char status[], char indicator) {
    jobs[job_count].job_id = job_count + 1;
    jobs[job_count].pid = pid;
    strcpy(jobs[job_count].command, command);
    strcpy(jobs[job_count].status, status);
    jobs[job_count].indicator = indicator;
    job_count++;
}

void removeJob(int idx) {
	if (idx < job_count) {
		while (idx + 1 < job_count) {
			jobs[idx+1].job_id--;
			jobs[idx] = jobs[idx+1];
			idx++;
		}
		jobs[idx] = jobs[idx+1];
		job_count--;
	}
}

void backToForeground(int job_id) {
    if (job_id < 1 || job_id > job_count) {
        printf("No such job\n");
        return;
    }

    // Set the foreground process
	int status;
    pid_t pid = jobs[job_id - 1].pid;
    if (pid == foreground_pid) {
        printf("Job %d is already in the foreground\n", job_id);
        return;
    }
    foreground_pid = jobs[job_id - 1].pid;

    // Resume the process if stopped
    kill(pid, SIGCONT);
    printf("%s", jobs[job_id - 1].command);
    // Wait for it to finish or stop again
    waitpid(pid, &status, WUNTRACED);

    // Check termination status
    if (WIFEXITED(status)) {
        strcpy(jobs[job_id - 1].status, "Done");
    } else if (WIFSTOPPED(status)) {
        strcpy(jobs[job_id - 1].status, "Stopped");
    }
}

void createBackgroundJob(char buffer[]) {
	pid_t pid;
	
	pid = fork();
	if  (pid < 0) {
		perror("Fork failed\n");
		exit(errno);
		return;
	}
	else if (!pid) {
		setpgid(0, 0);
		char *prog_argv[MAX_CMD_BUFFER];
		buildArgv(buffer, prog_argv);
		execvp(prog_argv[0], prog_argv);
		
		perror("Bad command\n");
		exit(errno);
	}
	else {
		setpgid(pid, pid);
		addJob(pid, ori_buffer, "Running", '\0');
		printf("[%d] %d\n", jobs[job_count-1].job_id, jobs[job_count-1].pid);
	}
}

void jobCmd() {
	if (job_count == 0) {
		printf("No job available\n");
	}
	int idxToRemove[MAX_CMD_BUFFER];
	for (int i = 0; i < job_count; i++) {
		printf("[%d]%c  %s\t %s", jobs[i].job_id, jobs[i].indicator, jobs[i].status, jobs[i].command);
	}
	int j = 0;
	while (idxToRemove[j] != '\0') {
		removeJob(j++);
		for (int i = j; j < job_count; j++) { idxToRemove[i]--; }
	}
}