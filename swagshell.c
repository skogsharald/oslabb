#define _GNU_SOURCE /* Not good at all, but necessary because of reasons. */

#include "swagshell.h"
#define COMMAND_CD "cd"
#define COMMAND_EXIT "exit"
#define COMMAND_CHECKENV "checkEnv"
#define MAX_LENGTH 80 /* As specified by instructions */
#define MAX_PARAMETERS 50 
/* Even if every command and parameter was 1 character long, 
the number of parameters would never exceed this as max length of input buffer*/

int main() {

	char cmd [MAX_LENGTH]; /* Input buffer */
	char *params [MAX_PARAMETERS]; /* Collection of command and parameters */
	int argc;
	int res;
	#if SIGDET > 0
		struct sigaction sa;
		printf("*** Running with SIGDET. ***\n");
	#else
		pid_t child_pid;
		printf("*** Running with polling. ***\n");
	#endif
	/* Register a signal handler for SIGINT, or ctrl-C, so this does not terminate shell */
	signal(SIGINT, handle_sigterm);
	/* Main loop of shell */
	while(1) {
		#if SIGDET > 0
			/* 
				Create a signal handler for SIGCHLD to print when a 
				background process finishes immediately after it does
			*/
			sa.sa_sigaction = &handle_sigchld;
			res = sigemptyset(&sa.sa_mask);
			if(res < 0) {
				perror("Problem setting up SIGCHLD handler.");
			}
			/* 
				SA_SIGINFO gives us some extra info about the process which sent the signald.
				SA_NOCLDSTOP makes sure SIGCHLD is not generated when children are stopped or continues.
				This is harmless because we only handle terminated processes and prevents the handler from
				being invoked unneccessarily.
			*/
			sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;
			if (sigaction(SIGCHLD, &sa, 0) < 0) {
				perror("Could not assign sigaction.");
				exit(EXIT_FAILURE); /* Something has seriously gone wrong. Exit. */
			}
		#else
			
			/* Poll and see if any child processes have been terminated */
			while ((child_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
				printf("%d terminated\n", child_pid);
		    }
		#endif
		/* Print prompt */
		fprintf(stdout, "swag > ");
		res = fflush(stdout);
		if(res < 0) {
			perror("Error flushing stdout");
		}
		/* Read from stdin and store in cmd */
		if(fgets(cmd, sizeof(cmd), stdin) == NULL)
		{
			/* 
				This bit handles the case of ctrl-D, upon which the prompt would be NULL.
				This makes sure prompt does not crash
			*/
			continue;
		}
		/* Parse out the command and arguments and pass on for execution*/
		argc = parseCmd(cmd, params);
		executeCmd(params, argc);
	}
	return EXIT_SUCCESS;
}

/* Signal handler for sigterm, or ctrl-c*/
void handle_sigterm(){
	/* Flush stdin so the ctrl-c character (^C) is not mistaken for a command */
	int e;
	e = tcflush(fileno(stdin), TCIFLUSH);
	if(e < 0){
		perror("Error flushing stdin");
	}
	/* Print prompt again */
	fprintf(stdout, "\n");
	fprintf(stdout, "swag > ");
	fflush(stdout);
}

/* Signal handler for sigchld, implemented to be used with sigaction instead of signal */
void handle_sigchld(int signal, siginfo_t *info, void *swagpointer){
	/* 
		Check the status of the pid in the signal info struct, 
		the zero means we're not saving the status (just checking it),
		and WNOHANG means waitpid does not hang main thread.
	*/
	while (waitpid(info->si_pid, 0, WNOHANG) > 0) {
		printf("%d terminated\n", info->si_pid);
	}
}

/* Call built-in chdir subroutine to change the directory*/
void changeDir(char *dir) {
	char cwd[1024];
	char *status_dir;
	int status;
	/* Get current working directory and print to user */
	status_dir = getcwd(cwd, sizeof(cwd));
	if(status_dir == NULL) {
		perror("Error retrieving cwd.");
	}
	printf("Changing from cwd: %s\n", &cwd[0]);
	/* Change directory */
	status = chdir(dir);
	if(status < 0) {
		perror("Error changing directory.");
	}
	/* Get new current working directory and print to user */
	status_dir = getcwd(cwd, sizeof(cwd));
	printf("Current working directory: %s\n", &cwd[0]);
	if(status_dir == NULL) {
		perror("Error retrieving cwd.");
	}
}

/* 
	Parse input and trim newlines.
	Saves command and its parameters in params.
	Returns argc.

*/
int parseCmd(char *cmd, char **params) {
	int i = 0;
	/* Tokenize string and iterate to split command and parameters from each other*/
	char * comm;
	comm = strtok(cmd, " \n");
	while(comm != NULL) {
		params[i] = comm;
		i++;
		comm = strtok(NULL, " \n");
	}
	params[i] = NULL;
	/* Return argc */
	return i;
}

/* executeBuiltIn executes built-in commands available to c through execvp*/
void executeBuiltIn(char **params, int argc) {
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
	} else if(pid == 0) {
		res = execvp(params[0], params);
		if(res < 0) {
			perror("Unknown command");
		}
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
	}
}

void checkEnv(char **params, int argc){
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
	my_pipe(cmd);
}

void my_pipe(char ***cmd) {
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
			perror("Pipe failed");
			return;
		if ((pid = fork()) == -1)
		{
			perror("Fork failed");
			return;
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
}

/* Executes what is saved in params */
void executeCmd(char **params, int argc){
	int res = 0;
	/*
	char cd_string [MAX_LENGTH] = COMMAND_CD;
	char exit_string [MAX_LENGTH] = COMMAND_EXIT;
	char checkenv_string [MAX_LENGTH] = COMMAND_CHECKENV;*/
	if(argc < 1){
		/* Handle empty commands */
		return;
	}
	if(strcmp(params[0], COMMAND_CD)==0){
		/* Only "cd" means to homedirectory */
		if (argc == 1)
			changeDir(getenv("HOME"));
		else if (argc == 2) /* Otherwise, we are cd'ing to a directory */
			changeDir(params[1]);
	}else if(strcmp(params[0], COMMAND_EXIT)==0){
		res = kill((pid_t) 0, SIGTERM); /* Send SIGTERM to all processes this process has started */
		/* If kill succeeded, we exit the shell */
		if(res > -1)
			exit(EXIT_SUCCESS);
		else /* Otherwise, error */
			perror("Could not terminate");
	}else if(strcmp(params[0], COMMAND_CHECKENV)==0){
		checkEnv(params, (argc-1));
	}else {
		executeBuiltIn(params, argc);
	}
}




