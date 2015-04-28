#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Declaration of the changeDir function */
int changeDir(char *);

int parseCmd(char*, char **);

int executeCmd(char **, int);

int executeBuiltIn(char **);

int checkEnv(char **, int);

int my_pipe(char ***);

void intHandler();
