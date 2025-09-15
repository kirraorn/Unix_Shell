/*
In Bash, tilde (~) may appear at the beginning of a path, and it expands to the environment variable $HOME.
 For example, tokens [“ls”, “~/dir1”] should expand to [“ls”, “/home/grads/mnguyen/dir1”].
 An output example of tilde expansion is:

~ → /home/grads/mnguyen
You will only have to handle the tilde expansion of a token if "~" a standalone or if it begins with "~/".

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
