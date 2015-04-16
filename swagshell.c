#include "swagshell.h"
#define COMMAND_CD "cd"
#define COMMAND_EXIT "exit"
#define COMMAND_CHECKENV "checkEnv"
#define MAX_LENGTH 80
#define MAX_PARAMETERS 10 /* More parameters than 10 will overwrite memory */
int main() {

	char cmd [MAX_LENGTH];
	char *params [MAX_PARAMETERS];
	while(1) {
		printf("swag > ");

		if(fgets(cmd, sizeof(cmd), stdin) == NULL) break;
		parseCmd(cmd, params);
		executeCmd(*params);


	}
	return EXIT_SUCCESS;
}

int changeDir(const char *dir) {
	return chdir(dir);
}

void parseCmd(char *cmd, char **params) {
	int i = 0;
	char * comm;
	comm = strtok(cmd, " ");
	while(comm != NULL) {
		params[i] = comm;
		i++;
		comm = strtok(NULL, " ");
	}
}

void executeCmd(const char *params){
	/*
	char cd_string [MAX_LENGTH] = COMMAND_CD;
	char exit_string [MAX_LENGTH] = COMMAND_EXIT;
	char checkenv_string [MAX_LENGTH] = COMMAND_CHECKENV;*/
	if(strcmp(&params[0], COMMAND_CD)==0){
		printf("%s is CD\n", &params[0]);
	}else if(strcmp(&params[0], COMMAND_EXIT)==0){
		printf("%s is EXIT\n", &params[0]);
	}else if(strcmp(&params[0], COMMAND_CHECKENV)==0){
		printf("%s is CHECKENV\n", &params[0]);
	}else {
		printf("%s is UNKNOWN\n", &params[0]);
	}
}





