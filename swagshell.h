#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

/* Declarations of functions for swagshell */
void changeDir(char *);

int parseCmd(char*, char **);

void executeCmd(char **, int);

void executeBuiltIn(char **, int);

void checkEnv(char **, int);

void the_pipe(char ***);

void handle_sigterm();

void handle_sigchld(int, siginfo_t *, void *);

void handle_zombie_sigchld(int);
