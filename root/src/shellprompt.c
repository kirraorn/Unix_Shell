#define _GNU_SOURCE  // Fix the syntax error here
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

#ifndef MAX_INPUT
#define MAX_INPUT 1024
#endif

// Convert main to a function that prints the prompt
void print_prompt() {
    char hostname[HOST_NAME_MAX];
    char cwd[PATH_MAX];
    
    char *user = getenv("USER");
    getcwd(cwd, sizeof(cwd));
    gethostname(hostname, sizeof(hostname));
    printf("%s@%s:%s> ", user, hostname, cwd);
    fflush(stdout); // Make sure prompt appears immediately
}
