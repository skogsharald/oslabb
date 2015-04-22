#include "swagshell.h"
#define COMMAND_CD "cd"
#define COMMAND_EXIT "exit"
#define COMMAND_CHECKENV "checkEnv"
#define MAX_LENGTH 80
#define MAX_PARAMETERS 10 /* More parameters than 10 will overwrite memory */
int main() {

	char cmd [MAX_LENGTH];
	char *params [MAX_PARAMETERS];
	int argc;
	while(1) {
		printf("swag > ");

		if(fgets(cmd, sizeof(cmd), stdin) == NULL) break;
		argc = parseCmd(cmd, params);
		executeCmd(params, argc);
	}
	return EXIT_SUCCESS;
}

/* Call built-in chdir subroutine */
int changeDir(char *dir) {
	char cwd[1024];
	char *swag;
	int status;
	swag = getcwd(cwd, sizeof(cwd));
	printf("Current working directory: %s\n", &cwd[0]);
	status = chdir(dir);
	/*printf("%d", status);*/
	swag = getcwd(cwd, sizeof(cwd));
	printf("Current working directory: %s\n", &cwd[0]);
	if(swag == NULL) {
		printf("wat");
		status = -1;
	}
	return status;
}

/* Parse input and trim newlines */
int parseCmd(char *cmd, char **params) {
	int i = 0;
	char * comm;
	comm = strtok(cmd, " \n");
	while(comm != NULL) {
		params[i] = comm;
		i++;
		comm = strtok(NULL, " \n");
	}
	params[i] = NULL;
	return i;
}

int executeBuiltIn(char **params) {
	/*printf("%s", params[0]);*/
	int res;
	pid_t pid;
	pid = fork();
	if(pid < 0){
		perror("Fuck all");
		res = EXIT_FAILURE;
	} else if(pid == 0) {
		res = execvp(params[0], params);
	} else {
		wait(NULL);
		res = EXIT_SUCCESS;
	}

	if(res < 0)
	{
		perror("Error when calling execvp: ");
	}
	return res;
}

int executeCmd(char **params, int argc){
	int res;
	char *msg;
	/*
	char cd_string [MAX_LENGTH] = COMMAND_CD;
	char exit_string [MAX_LENGTH] = COMMAND_EXIT;
	char checkenv_string [MAX_LENGTH] = COMMAND_CHECKENV;*/
	if(strcmp(params[0], COMMAND_CD)==0){
		/*printf("%s is CD\n", &params[0]);*/
		if (argc == 1)
			res = changeDir(getenv("HOME"));
		else if (argc == 2)
			res =  changeDir(params[1]);
		else
			res =  EXIT_FAILURE;
		if(res < 1) {
			msg = "Error";
		}
	}else if(strcmp(params[0], COMMAND_EXIT)==0){
		printf("%s is EXIT\n", params[0]);
		res =  EXIT_SUCCESS;
	}else if(strcmp(params[0], COMMAND_CHECKENV)==0){
		printf("%s is CHECKENV\n", params[0]);
		res =  EXIT_SUCCESS;
	}else {
		res = executeBuiltIn(params);
		if(res < 0){
			msg = "Unknown command";
			/*printf("Unkown command: %s\n", &params[0]);*/
		}
	}
	if(res < 0) {
		perror(msg);
	}
	return res;
}





