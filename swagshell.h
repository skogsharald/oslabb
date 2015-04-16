#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* Declaration of the changeDir function */
int changeDir(const char*);

void parseCmd(char*, char **paramList);

void executeCmd(const char *params);
