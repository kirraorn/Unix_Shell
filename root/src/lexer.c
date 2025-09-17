#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int gethostname(char *name, size_t length);

void printPrompt()
{
        char *user = getenv("USER");
        char hostname[256];
        char cwd[4096];

        gethostname(hostname, sizeof(hostname));
        getcwd(cwd, sizeof(cwd));

        printf("%s@%s:%s> ", user, hostname, cwd);
}

int main()
{
	while (1) {
		printPrompt();

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

		tokenlist *tokens = get_tokens(input);
		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}
/*tilde expansion*/
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
