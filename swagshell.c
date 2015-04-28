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
		perror("Fork failed");
		res = EXIT_FAILURE;
	} else if(pid == 0) {
		res = execvp(params[0], params);
	} else {
		wait(NULL);
		res = EXIT_SUCCESS;
	}
	return res;
}

int checkEnv(char **params, int argc){


	char *printenv[2];
	char *grep[10];
	char *sort[2];
	char *pager[2];
	char **cmd[5];
	char* pager_env;
	int i;
	pager_env = getenv("PAGER");
	printenv[0] = "printenv";
	if(pager_env == NULL) {
		pager[0] = "less";
	} else {
		pager[0] = pager_env;
	}
	pager[1] = NULL;
	sort[0] = "sort";
	sort[1] = NULL;
	cmd[0] = printenv;
	if(argc > 0){
		grep[0] = "grep";
		for(i = 1; i <= argc; i++ ){
			grep[i] = params[i];
			grep[(i+1)] = NULL;
		}
		cmd[1] = grep;
		cmd[2] = sort;
		cmd[3] = pager;
	} else {
		
		cmd[1] = sort;
		cmd[2] = pager;
		cmd[3] = NULL;
	}
	cmd[4] = NULL;
	
	
	return my_pipe(cmd);
}

int my_pipe(char ***cmd) {
	int saved_in;
	int saved_out;
	int   p[2];
	pid_t pid;
	int   fd_in;
	int pipe_res;
	char *more[2];

	fd_in = 0;
	/* Hard-code and introduce more as an alternative to less */
	more[0] = "more";
	more[1] = NULL;
	/* Save STDIN and STDOUT */
	saved_in = dup(0);
	saved_out = dup(1);

	while (*cmd != NULL)
	{
		pipe_res = pipe(p);
		if(pipe_res < 0)
			return EXIT_FAILURE;
		if ((pid = fork()) == -1)
		{
			return EXIT_FAILURE;
		}
		else if (pid == 0) /* Child process */
		{
			dup2(fd_in, 0); /* First iteration, make sure we read from STDIN, otherwise read from whatever specified by fd_in */
			if (*(cmd + 1) != NULL){
				dup2(p[1], 1); /* If the next command in chain is not null, make sure we write to pipe write file descriptor */
			}
			close(p[0]); /* Close file descriptor for read end of pipe */
			execvp((*cmd)[0], *cmd); /*Execute command*/
			if(*(cmd + 1) == NULL ) {
				if(strcmp((*cmd)[0], "less") == 0){
					/* If we end up here, it means less has failed and thus we run more instead*/
					execvp(more[0], more);
				}
				/* Here, the pager has failed */
				perror("Could not use PAGER variable.");
			}
			exit(EXIT_FAILURE); /* Fuck all */
		}
		else
		{
			wait(NULL); /* Wait for child process */
			close(p[1]); /* Close the write end of pipe */
			fd_in = p[0]; /* Set fd_in to read end of pipe, so we in next iteration read from this file descriptor*/
			cmd++; /* Next command in chain */

		}
	}
	/* Restore STDIN and STDOUT to print */
	dup2(saved_in, 0);
	dup2(saved_out, 1);
	return EXIT_SUCCESS;
}

int executeCmd(char **params, int argc){
	int res;
	char *msg;
	/*
	char cd_string [MAX_LENGTH] = COMMAND_CD;
	char exit_string [MAX_LENGTH] = COMMAND_EXIT;
	char checkenv_string [MAX_LENGTH] = COMMAND_CHECKENV;*/
	if(argc < 1){
		return EXIT_SUCCESS;
	}
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
		res = checkEnv(params, (argc-1));
		if(res < 1){
			msg = "checkEnv failed";
		}
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




