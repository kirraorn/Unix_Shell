#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h> //sets limits for the host and path arrays
#include <string.h>

int main() {

char hostname[HOST_NAME_MAX]; //make arrays for the host and path names
char cwd[PATH_MAX];
char input[MAX_INPUT];

while (1) {
  char *user = getenv("USER"); //get username, cwd, and host name
  getcwd(cwd, sizeof(cwd));
  gethostname(hostname, sizeof(hostname));

  printf("%s@%s:%s> ", user, hostname, cwd); //print them out with formatting

  if (fgets(input, sizeof(input), stdin) == NULL) // get user input/ if using control D then just go to new line and exit
  {
      printf("\n");
      break;
  }

  input[strcspn(input, "\n")] = 0; // get rid of newline for future parsing

  if (strcmp(input, "exit") == 0) // if user actually types exit, then exit
  { break; }
}
  return 0;

}
