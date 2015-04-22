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

/* Call built-in chdir subroutine */
int changeDir(const char *dir) {
	char cwd[1024];
	int status;
	printf("%s", dir);
	getcwd(cwd, sizeof(cwd));
	printf("Current working directory: %s\n", &cwd[0]);
	status = chdir(dir);
	printf("%d", status);
	getcwd(cwd, sizeof(cwd));
	printf("Current working directory: %s\n", &cwd[0]);
	return status;
}

/* Parse input and trim newlines */
void parseCmd(char *cmd, char **params) {
	int i = 0;
	char * comm;
	comm = strtok(cmd, " \n");
	while(comm != NULL) {
		params[i] = comm;
		i++;
		comm = strtok(NULL, " \n");
	}
}

int executeBuiltIn(char params) {
	int res = execvp(params[0], params);
	if(res == NULL) {
		res = 1;
	}
	return res;
}

int executeCmd(const char *params){
	int res;
	char msg;
	/*
	char cd_string [MAX_LENGTH] = COMMAND_CD;
	char exit_string [MAX_LENGTH] = COMMAND_EXIT;
	char checkenv_string [MAX_LENGTH] = COMMAND_CHECKENV;*/
	if(strcmp(&params[0], COMMAND_CD)==0){
		/*printf("%s is CD\n", &params[0]);*/
		res = changeDir(&params[1]);
		if(res < 1) {
			msg = "Not a directory.";
		}
	}else if(strcmp(&params[0], COMMAND_EXIT)==0){
		printf("%s is EXIT\n", &params[0]);
		res =  EXIT_SUCCESS;
	}else if(strcmp(&params[0], COMMAND_CHECKENV)==0){
		printf("%s is CHECKENV\n", &params[0]);
		res =  EXIT_SUCCESS;
	}else {
		res = executeBuiltIn(params);
		if(res < 0){
			msg = "Unknown command";
			//printf("Unkown command: %s\n", &params[0]);
		}
	}
	if(res < 0) {
		perror(msg);
	}
	return res;
}





