/*
In Bash, tilde (~) may appear at the beginning of a path, and it expands to the environment variable $HOME.
 For example, tokens [“ls”, “~/dir1”] should expand to [“ls”, “/home/grads/mnguyen/dir1”].
 An output example of tilde expansion is:

~ → /home/grads/mnguyen
You will only have to handle the tilde expansion of a token if "~" a standalone or if it begins with "~/".

*/
/*
 check if the token starts with ~
	get the home directory-> getenv("HOME")
	Change ~ to the home directory path
	output new strings like /home/dorval/documents
*/
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tilde_expansion.h"
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char* expand_tilde(const char* token) {
	char* home = getenv("HOME");
	if (home == NULL){
		return strdup(token); //no home, return the original
	}

	//if the token is null or empty then return NULL
	if (token == NULL || strlen(token) == 0) {
		return NULL;
	}

	if (token[0] != '~') {//if the first character is not tilde, no expansion needed
		return strdup(token); //duplicate
	}
	else if (strlen(token)==1 && token[0]== '~'){//if the only input is ~
		return strdup(home);
	}
	else if (token[1] == '/'){
	//implemeny it
		size_t home_len = strlen(home);
        	size_t rest_len = strlen(token + 1);
    		size_t total = home_len + rest_len + 1;

		char* result = malloc(total);

		if (result == NULL){
		 return NULL;//in the case where memory allocation fails
		}

		strcpy(result, home);
    		strcat(result, token + 1);
    		return result;
	}//end of else if

	return strdup(token); //as default return the origina;

}//end of expand_tilde

void expand_tilde_in_tokens(char** tokens, int num_tokens) {
	if (tokens == NULL) { //if no tokens the funciton exits
        return;
        }

	for (int i = 0; i < num_tokens; i++) {//loop through the array, for each token
		if (tokens[i] != NULL) {//if the token is not null then expand it
			char* expanded = expand_tilde(tokens[i]);

			if (expanded != NULL && strcmp(expanded, tokens[i]) != 0) {
                		//the expansion happened, replace the token
                		free(tokens[i]);
                		tokens[i] = expanded;
            		} else if (expanded != NULL) {
                		//no expansion, but we got a copy, free it
                		free(expanded);
			}
        	}
	}
}//end of expand_tilde_in tokens
