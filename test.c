#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stddef.h>

#include <sys/wait.h>

#include <sys/ioctl.h>

#include <sys/termios.h>


/* NOTE: This example illustrates tcsetgrp() and setpgrp(), but doesn’t function correctly because SIGTTIN and SIGTTOU aren’t handled.*/


int main()

{

 int status;

 int cpid;

 int ppid;

 char buf[256];

 sigset_t blocked;


 ppid = getpid();



if (!(cpid=fork()))

 {

   setpgid(0,0);

   tcsetpgrp (0, getpid());

   execl ("/bin/vi", "vi", NULL);

   exit (-1);

 }

 

 if (cpid < 0)

   exit(-1);



 setpgid(cpid, cpid);

 tcsetpgrp (0, cpid);


 waitpid (cpid, NULL, 0);


 tcsetpgrp (0, ppid);


 while (1)

 {

   memset (buf, 0, 256);

   fgets (buf, 256, stdin);

   puts ("ECHO: ");

   puts (buf);

   puts ("\n");

 }

}

