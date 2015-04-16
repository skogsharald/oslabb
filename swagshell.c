#include "swagshell.h"

#define COMMAND_CD "cd"
#define COMMAND_EXIT "exit"
#define COMMAND_CHECKENV "checkEnv"
#define MAX_LENGTH 80
int main() {
	char cd_string [MAX_LENGTH] = COMMAND_CD;
	char exit_string [MAX_LENGTH] = COMMAND_EXIT;
	char checkenv_string [MAX_LENGTH] = COMMAND_CHECKENV;

	char cmd [MAX_LENGTH];
	while(1) {
		printf("swag > ");

		scanf("%[^\n]%*c", cmd);
		if(&cmd[0] == NULL) break;





if(strcmp(cmd,cd_string)==0){
	printf("%s is CD\n", &cmd[0]);
}else if(strcmp(cmd,exit_string)==0){
	printf("%s is EXIT\n", &cmd[0]);
}else if(strcmp(cmd,checkenv_string)==0){
	printf("%s is CHECKENV\n", &cmd[0]);
}else {
	printf("%s is UNKNOWN\n", &cmd[0]);
}




		
	}
	return EXIT_SUCCESS;
}

int changeDir(const char *dir) {
	return chdir(dir);
}
