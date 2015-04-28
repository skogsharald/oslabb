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

int checkEnv(char **params){


	char *ls[] = {"printenv", NULL};
  	char *grep[] = {"sort", NULL};
  	char *wc[] = {NULL};
  	char **cmd[] = {ls, grep, wc, NULL};

  
  loop_pipe(cmd);
  return 0;
}

void    loop_pipe(char ***cmd) {
	int   p[2];
  pid_t pid;
  int   fd_in = 0;

  while (*cmd != NULL)
    {

      printf("Command in loop was: %s", *cmd);
      pipe(p);
      if ((pid = fork()) == -1)
        {
          exit(EXIT_FAILURE);
        }
      else if (pid == 0) /* Child process */
      {
          dup2(fd_in, 0); /* First iteration, make sure we read from STDIN, otherwise read from whatever specified by fd_in */
          if (*(cmd + 1) != NULL)
            dup2(p[1], 1); /* If the next command in chain is not null, make sure we write to pipe write file descriptor */
          close(p[0]); /* Close file descriptor for read end of pipe */
          execvp((*cmd)[0], *cmd); /*Execute command*/
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
  }


/*
	int res;
	int fds[2];
	pipe(fds);

	//res = execlp("printenv", "printenv", NULL);
	
	pipe(fds);
	pid_t pid = fork();

	if(pid==0){
		printf("parent pid is: %i", pid);
		close(fds[1]);
		dup2(fds[0], STDIN_FILENO);
		

		wait(pid);
		printf("WAIT COMPLETE");
		pid_t pid2 = fork();
		int fds2[2];

		if(pid==0){
			printf("parent pid is: %i", pid);
			close(fds2[1]);
			dup2(fds2[0], STDIN_FILENO);  
		}else if(pid > 0){
			close(fds2[0]);
			printf("child2 pid is: %i", pid);
			dup2(fds2[1], STDOUT_FILENO);
			execlp("printenv", NULL);
			close(fds2[1]);
			
		}else{
			printf("didn't work... pid is: %i", pid);
			execvp("sort", NULL);
	}
*/

/*
	}else if(pid > 0){
		close(fds[0]);
		printf("child pid is: %i", pid);
		dup2(fds[1], STDOUT_FILENO);
		execlp("printenv", NULL);
		close(fds[1]);
		

	}else{
		
		printf("didn't work... pid is: %i", pid);
		execvp("sort", NULL);
		
	}
	*/





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
		res = checkEnv(params);
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




