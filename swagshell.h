#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Declaration of the changeDir function */
int changeDir(char *);

int parseCmd(char*, char **);

int executeCmd(char **, int);

int executeBuiltIn(char **);
