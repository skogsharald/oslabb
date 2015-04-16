#include "swagshell.h"

int main(int argc, char *argv[]) {
	int status = changeDir(argv[1]);
	printf("%d", status);
	printf("%s", argv[2]);
	return status;
}

int changeDir(const char *dir) {
	return chdir(dir);
}
