#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CMD_BUFFER 255

typedef struct {
    int job_id;
    pid_t pid;
    char command[MAX_CMD_BUFFER];
    char status[10];
    char indicator;
} Job;