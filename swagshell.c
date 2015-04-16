#include "swagshell.h"

#define MAX_LENGTH 80
int main() {
	char cmd [MAX_LENGTH];
	while(1) {
		printf("swag > ");

		scanf("%[^\n]%*c", cmd);
		if(cmd[0] == NULL) break;
		printf("%s", &cmd[0]);
	}
	return EXIT_SUCCESS;
}

int changeDir(const char *dir) {
	return chdir(dir);
}
