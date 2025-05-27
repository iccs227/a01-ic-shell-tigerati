/* ICCS227: Project 1: icsh
 * Name: Athiwat Panboonprung
 * StudentID: 6680729
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "execscript.h"
#include "exec.h"

int main(int argc, char* argv[]) {
	printf("Starting IC Shell\n");
	if (argc == 2) { 
		runWithFile(argv);
	}
	else { runWithOutFile(argv); }
	return 0;
}