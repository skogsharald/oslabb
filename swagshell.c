#define _GNU_SOURCE /* Not good at all, but necessary because of reasons. */

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
	#if SIGDET > 0
		struct sigaction sa;
		printf("*** Running with SIGDET. ***\n");
	#else
		pid_t child_pid;
		printf("*** Running with polling. ***\n");
	#endif
	signal(SIGINT, intHandler);
	while(1) {
		#if SIGDET > 0
			
			sa.sa_sigaction = &handle_sigchld;
			sigemptyset(&sa.sa_mask);
			sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
			if (sigaction(SIGCHLD, &sa, 0) < 0) {
				perror(0);
				exit(-1); /* Something has seriously gone wrong. Exit. */
			}
		#else
			
			/* Poll and see if any child processes have been terminated */
			while ((child_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
				printf("%d terminated\n", child_pid);
		    }
		#endif
		fprintf(stdout, "swag > ");
		fflush(stdout);
		if(fgets(cmd, sizeof(cmd), stdin) == NULL)
		{
			continue;
		}
		argc = parseCmd(cmd, params);
		executeCmd(params, argc);
	}
	return EXIT_SUCCESS;
}

void intHandler(){
	/*fprintf(stderr, "\nswag > ");*/
	int e;
	e = tcflush(fileno(stdin), TCIFLUSH);
	if(e < 0){
		perror("Error flushing stdin");
	}
	fprintf(stdout, "\n");
	fprintf(stdout, "swag > ");
	fflush(stdout);
}

void handle_sigchld(int signal, siginfo_t *info, void *swagpointer){
	while (waitpid(info->si_pid, 0, WNOHANG) > 0) {
		printf("%d terminated\n", info->si_pid);
	}
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

int executeBuiltIn(char **params, int argc) {
	/*printf("%s", params[0]);*/
	int res;
	pid_t pid;
	int background;
	int i;
	int start_sec, end_sec, start_usec, end_usec;
	struct timeval t1, t2;
	double elapsedTimeMillis;
	background = 0;
	for(i = 0; i < argc; i++){
		if(strcmp(params[i], "&") == 0){
			background = 1;
			params[i] = NULL;
		}
	}
	pid = fork();
	if(pid < 0){
		perror("Fork failed");
		res = EXIT_FAILURE;
	} else if(pid == 0) {
		res = execvp(params[0], params);
	} else {
		if(background == 0){
			/* Measure time if */
			gettimeofday(&t1, NULL);
			wait(NULL);
			gettimeofday(&t2, NULL);
			start_sec = t1.tv_sec;
			end_sec = t2.tv_sec;
			start_usec = t1.tv_usec;
			end_usec = t2.tv_usec;

			elapsedTimeMillis = (end_sec * 1000.0 + end_usec / 1000.0) - (start_sec * 1000.0 + start_usec / 1000.0);
			fprintf(stdout, "Time taken %f ms\n", elapsedTimeMillis);
		}
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
		res = kill((pid_t) 0, SIGTERM); /* Send SIGTERM to all processes this process has started*/
		if(res > -1)
			exit(EXIT_SUCCESS);
		else
			msg = "Could not Terminate";
	}else if(strcmp(params[0], COMMAND_CHECKENV)==0){
		res = checkEnv(params, (argc-1));
		if(res < 1){
			msg = "checkEnv failed";
		}
	}else {
		res = executeBuiltIn(params, argc);
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




