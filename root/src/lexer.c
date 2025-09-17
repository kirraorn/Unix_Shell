#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* part one: print prompt*/
int gethostname(char *name, size_t length); // use this to get the host name 

void printPrompt() //prints prompt out
{
        char *user = getenv("USER"); // create user, hostname and cwd arrays
        char hostname[256];
        char cwd[4096];

        gethostname(hostname, sizeof(hostname)); // get the hostname and the cwd
        getcwd(cwd, sizeof(cwd));

        printf("%s@%s:%s> ", user, hostname, cwd); //print out prompt with unique name
}
/* end of part one: print prompt */
/* part 4: $PATH SEARCH */

char *findPath(char *command)
{

        if (strchr(command, '/')) // if theres already a path then just return
        { return strdup(command); }

        char *path = getenv("PATH"); // gives me the full path
        char *path_copy = strdup(path); // so strtok doesn't mess it up
        char *folder = strtok(path_copy, ":");
        static char full_path[4096]; //save full path (static so it survives past function)

        while (folder != NULL) // go through each folder that was made from strtok
        {
                snprintf(full_path, sizeof(full_path), "%s/%s", folder, command); //print path  

                if (access(full_path, X_OK)  == 0) // if executable is there then return and free poin>
                {
                        free(path_copy);
                        return strdup(full_path);
                }
                folder = strtok(NULL, ":"); // now folder can go to next folder and repeat
        }
        free(path_copy); // free memory and if not found then return null
        return NULL;
/* end of part 4: PATH SEARCH */
	
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
		if (tokens->size > 0) // if user entered something
        {

        	char *command_path = findPath(tokens->items[0]); // run function to get path
			if (command_path) // if exists, print then free the pointer
            {
                    printf ("found: %s\n", command_path);
                    free(command_path);
            }
            else // if not, prein error
                { printf("%s: command not found\n", tokens->items[0]); }
        }


		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		 //LD adding
			// 1. Expand tilde in tokens
        expand_tilde_in_tokens(tokens->items, tokens->size);
        
        // 2. Parse redirection
        char *input_filename, *output_filename;
        bool has_input_flag, has_output_flag;
        parse_redirection(tokens->items, tokens->size, &input_filename, &output_filename, 
                         &has_input_flag, &has_output_flag);
        
        // 3. Remove redirection tokens
        int new_size = remove_redirection_tokens(tokens->items, tokens->size);
        
        // 4. Execute the command
        if (new_size > 0) {
            execute_command(tokens->items, input_filename, output_filename, 
                           has_input_flag, has_output_flag);
        }
		//end of adding
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

/*io redirection*/
/*######################################################################################*/
void parse_redirection(char *tokens[], int num_tokens, char **input_filename, char **output_filename, bool *has_input_flag, bool *has_output_flag) {
    // Initialize flags and filenames
    *has_input_flag = false;
    *has_output_flag = false;
    *input_filename = NULL;
    *output_filename = NULL;

    // Iterate over tokens
    for (int i = 0; i < num_tokens; i++) {
        //if there is the sign for an input in the token
        if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 < num_tokens) {//if there is a token after it
                *input_filename = tokens[i + 1];//set the input file name to be the following token
                *has_input_flag = true; //change the flag to true
            }
        //else if there is the sign for an output, do the same from input but for output
        } else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 < num_tokens) {
                *output_filename = tokens[i + 1];
                *has_output_flag = true;
            }
        }
    }
}

/*######################################################################################*/
int remove_redirection_tokens(char *tokens[], int num_tokens) {
    int write_index = 0;//keeps trac o the position in the list where we put tokens we want to keep

    for (int read_index = 0; read_index < num_tokens;) {//go through every token in the list
        //check for redirection symbol
        if (strcmp(tokens[read_index], "<") == 0 || strcmp(tokens[read_index], ">") == 0) {
            read_index += 2; //skip the symbol and the file name
        } else {
        /*
        take the token at the position where we are reading and put it where we want to keep the
        tokens. then move forward by 1
        */
            tokens[write_index++] = tokens[read_index++];//copy current token
        }
    }
    return write_index;//return the new number of tokens after removing redirection tokens
}
/*######################################################################################*/
int setup_redirection(const char *input_filename, const char *output_filename,
                      bool has_input_flag, bool has_output_flag) {
    int fd;//number to keep track of the files open

    //if there is an input
    if (has_input_flag) {
        fd = open(input_filename, O_RDONLY); //open file for read only
        if (fd == -1) {
            perror("Failed to open input file");
            return -1;
        }
        //input redirection
        if (dup2(fd, STDIN_FILENO) == -1) {//if it failed to open
            perror("Failed to redirect stdin");
            close(fd);
            return -1;
        }
        close(fd);
    }

    //if there is an output
    if (has_output_flag) {
        /*open for write onl, create if it doe not exist, erase file content if it's already there*/
        fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd == -1) {
            perror("Failed to open output file");
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            perror("Failed to redirect stdout");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}
/*######################################################################################*/
int execute_command(char *tokens[], const char *input_filename, const char *output_filename,
                    bool has_input_flag, bool has_output_flag) {
        /*
        create a new process(child), sets up redirection, run command
        */
    pid_t pid = fork();//make a copy of the curren process

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {//child process
        if (setup_redirection(input_filename, output_filename, has_input_flag, has_output_flag) != 0) {
            exit(1);//from our previous function, if it did not return 0, then quit
        }

        execv(tokens[0], tokens);//replace the child iage with the new command

        perror("execv failed");//if it fails, print error and quit
        exit(1);
    } else {//the parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {//if it failed waiting for the child prcess to end,quit
            perror("waitpid failed");
            return -1;
        }

        if (WIFEXITED(status)) {//returns trye if child terminated normally
            return WEXITSTATUS(status);//retrurn the child exit code
        } else {
            fprintf(stderr, "Child did not exit normally\n");
            return -1;
        }
    }
}
/*end of io redirection*/
