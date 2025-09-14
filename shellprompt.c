#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h> //sets limits for the host and path arrays

int main() {

char hostname[HOST_NAME_MAX]; //make arrays for the host and path names
char cwd[PATH_MAX];

char *user = getenv("USER"); //get username, cwd, and host name
getcwd(cwd, sizeof(cwd));
gethostname(hostname, sizeof(hostname));

printf("%s@%s:%s> ", user, hostname, cwd); //print them out with formatting

return 0;

}
