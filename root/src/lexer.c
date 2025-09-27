#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* part one: print prompt*/
void printPrompt() //prints prompt out
{
        char *user = getenv("USER"); // create user, hostname and cwd arrays
        char hostname[256];
        char cwd[4096];

        gethostname(hostname, sizeof(hostname)); // get the hostname and the cwd
        getcwd(cwd, sizeof(cwd));

        printf("%s@%s:%s> ", user, hostname, cwd); //print out prompt with unique name
		fflush(stdout);
}
/* end of part one: print prompt */

/*part 2 */
void replace_env_tokens(tokenlist *tokens) 
{
    for (int i = 0; i < tokens->size; i++) 
	{
        char *tok = tokens->items[i];
        if (tok[0] == '$' && tok[1] != '\0') 
		{
            const char *val = getenv(tok + 1);
            free(tokens->items[i]);
            tokens->items[i] = strdup(val ? val : "");
        }
    }
}
/* end of part 2 */ 



/* part 4: $PATH SEARCH */

char *findPath(char *command)
{
		if (!command) // if not a command then exit
			return NULL;

        if (strchr(command, '/')) // if theres already a path then just return
        { 
			if (access(command, X_OK) == 0)
			{ return strdup(command); }
			return NULL;
		}

        char *path = getenv("PATH"); // gives me the full path
		if (!path) 
		{ return NULL; }
	
        char *path_copy = strdup(path); // so strtok doesn't mess it up
        char *folder = strtok(path_copy, ":");

        while (folder != NULL) // go through each folder that was made from strtok
        {	
				char full_path[4096];
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
}
/* end of part 4: PATH SEARCH */

/* part 9: command history for exit */
char *command_history[3] = {NULL, NULL, NULL};
int history_count = 0;

void add_to_history(char *command) {
    if (command_history[0]) free(command_history[0]);
    command_history[0] = command_history[1];
    command_history[1] = command_history[2];
    command_history[2] = strdup(command);
    if (history_count < 3) history_count++;
}
/* end part 9 history */

//LD added for testing - Part 8 job tracking
typedef struct {
    int job_number;
    pid_t pid;
    char *command;
} BackgroundJob;

BackgroundJob bg_jobs[10];
int bg_job_count = 0;
//LD end addition
//LD added for testing - function declaration
int run_in_background(char *tokens[], const char *input_filename, const char *output_filename,
                      bool has_input_flag, bool has_output_flag, int job_number);
//LD end addition
int main()
{
	int job_count = 1;  // start background jobs from 1
	while (1) {
		printPrompt();

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
//printf("whole input: %s\n", input);
		char *nl = strchr(input, '\n');  // get rid of any newline if there
        if (nl) 
			{ *nl = '\0'; }
		
		tokenlist *tokens = get_tokens(input);
		replace_env_tokens(tokens); // part 2: expand environment variables
		
		//LD added for testing - check for background
		bool is_background = false;
		if (tokens->size > 0 && strcmp(tokens->items[tokens->size - 1], "&") == 0) {
			is_background = true;
			free(tokens->items[tokens->size - 1]);
			tokens->items[tokens->size - 1] = NULL;
			tokens->size--;
		}
		//LD end addition
		
		if (tokens->size > 0) // if user entered something
        {
			// part 9: add to history
			add_to_history(input);
			
			//check if there is a pipe
            int num_commands = 1;
            for (int i = 0; i < tokens->size; i++)
            {
                if (strcmp(tokens->items[i], "|") == 0)
                    { num_commands++; }
            }

            if (num_commands > 1)
            {
                char **commands[3]; //hold up to three commands
                int token_index = 0;

                for (int i= 0; i < num_commands; i++)
                {
                    commands[i] = malloc((tokens->size + 1) * sizeof(char *));
                    int arg_index = 0;

                    while (token_index < tokens->size && strcmp(tokens->items[token_index], "|") != 0)
					{
                        commands[i][arg_index++] = tokens->items[token_index];
                        token_index++;
                    }
                        commands[i][arg_index] = NULL; // terminate argv

						if (token_index < tokens->size && strcmp(tokens->items[token_index], "|") == 0)
                        	token_index++; // skip "|"
                }

                piping(commands, num_commands);

            	for (int i = 0; i < num_commands; i++)
                    { free(commands[i]); }

                }
/* part 5 */
               else // if not a pipe
				{
				 /* PART 9: BUILT-IN COMMANDS */
				 if (strcmp(tokens->items[0], "exit") == 0) {
					 // Wait for background jobs
					 //LD added for testing
					 for (int i = 0; i < bg_job_count; i++) {
						 if (bg_jobs[i].pid > 0) {
							 waitpid(bg_jobs[i].pid, NULL, 0);
							 free(bg_jobs[i].command);
						 }
					 }
					 //LD end addition
					 
					 if (history_count == 0) {
						 printf("No commands in history.\n");
					 } else {
						 printf("Last %d command(s):\n", history_count);
						 for (int i = 0; i < history_count; i++) {
							 if (command_history[i]) {
								 printf("%d: %s\n", i+1, command_history[i]);
							 }
						 }
					 }
					 
					 for (int i = 0; i < 3; i++) {
						 if (command_history[i]) free(command_history[i]);
					 }
					 
					 free(input);
					 free_tokens(tokens);
					 exit(0);
				 }
				 else if (strcmp(tokens->items[0], "cd") == 0) {
					 if (tokens->size > 2) {
						 fprintf(stderr, "cd: too many arguments\n");
					 }
					 else if (tokens->size == 1) {
						 char *home = getenv("HOME");
						 if (chdir(home) != 0) {
							 perror("cd");
						 }
					 }
					 else {
						 if (chdir(tokens->items[1]) != 0) {
							 perror("cd");
						 }
					 }
				 }
				 else if (strcmp(tokens->items[0], "jobs") == 0) {
					 //LD added for testing
					 if (bg_job_count == 0) {
						 printf("No active background jobs\n");
					 } else {
						 for (int i = 0; i < bg_job_count; i++) {
							 if (bg_jobs[i].pid > 0) {
								 printf("[%d]+ %d %s\n", bg_jobs[i].job_number, bg_jobs[i].pid, bg_jobs[i].command);
							 }
						 }
					 }
					 //LD end addition
				 }
				 else {
					 // Single command path
					 expand_tilde_in_tokens(tokens->items, tokens->size);

					 // Parse redirection
					 char *input_filename, *output_filename;
					 bool has_input_flag, has_output_flag;
					 parse_redirection(tokens->items, tokens->size,
									   &input_filename, &output_filename,
									   &has_input_flag, &has_output_flag);

					 // Remove redirection tokens
					 int new_size = remove_redirection_tokens(tokens->items, tokens->size);
					 tokens->items[new_size] = NULL; // NULL terminate for execv

					 // Execute command
					 if (new_size > 0) {
						 //LD added for testing - background check
						 if (is_background) {
							 int pid = run_in_background(tokens->items, input_filename, output_filename, has_input_flag, has_output_flag, job_count);
							 bg_jobs[bg_job_count].job_number = job_count++;
							 bg_jobs[bg_job_count].pid = pid;
							 bg_jobs[bg_job_count].command = strdup(input);
							 bg_job_count++;
						 } else {
							 execute_command(tokens->items, input_filename, output_filename, has_input_flag, has_output_flag);
						 }
						 //LD end addition
					 }
				 }
				}
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
		buffer = (char *)realloc(buffer, bufsize + addby + 1);
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
		char *full_path = findPath(tokens[0]);
        if (!full_path) {
            fprintf(stderr, "%s: command not found\n", tokens[0]);
            exit(1);
        }
        execv(full_path, tokens);
        
        perror("execv failed");//if it fails, print error and quit
        free(full_path);
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

/* part 7: piping */

int piping(char  **commands[], int num_commands)
{
        int pipefds[4]; // make enough for 2  pipes with ends
        pid_t pids[3]; // store pids of the processes

        if (num_commands > 1 && pipe(pipefds) == -1) //if not one pipe, fail
        { printf("pipe not able to run\n"); return -1;}

        if (num_commands > 2  && pipe(pipefds + 2) == -1) // if not two pipes, fail
        { printf("pipe unable to run\n"); return -1; }

        for (int i = 0; i < num_commands; i++) //for each command...
        {
                if ((pids[i] = fork()) == 0) // if the pid matches the fork
                {
                        if (i == 0) // if its the firs command, redirect stout to end of pipe1
                        { dup2(pipefds[1], STDOUT_FILENO); }
                        else if (i == num_commands - 1)
                        { dup2(pipefds[2*(i-1)], STDIN_FILENO); } // if last command, get read end of last pipe and add 
                        else // if three commands, put the read end of the previous pipe to the write of the next
                        {
                                dup2(pipefds[2*(i-1)], STDIN_FILENO);
                                dup2(pipefds[2*i+1], STDOUT_FILENO);
                        }

                        for (int j = 0; j < 2*(num_commands - 1); j++) // close the child pipes to make sure its not blocked
                                close(pipefds[j]);

						char *full_path = findPath(commands[i][0]);
                		if (!full_path) { fprintf(stderr, "%s: command not found\n", commands[i][0]); exit(1); }

             			execv(full_path, commands[i]);
            			perror("execv failed");
                		free(full_path);
						exit(1);
                }
        }

        for (int i = 0; i < 2*(num_commands -1); i++) // again, close the child pipes left
                close(pipefds[i]);

        for (int i = 0; i < num_commands; i ++) // make sure all parent processes wait to finish until children are done
                waitpid(pids[i], NULL, 0);

return 0;

}
/* end of piping */
/* part 8: background processing */

int run_in_background(char *tokens[], const char *input_filename, const char *output_filename,
                      bool has_input_flag, bool has_output_flag, int job_number) 
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) { 
        // Child process
        if (setup_redirection(input_filename, output_filename, has_input_flag, has_output_flag) != 0) {
            exit(1);
        }

        char *full_path = findPath(tokens[0]);
        if (!full_path) {
            fprintf(stderr, "%s: command not found\n", tokens[0]);
            exit(1);
        }

        execv(full_path, tokens);
        perror("execv failed");
        free(full_path);
        exit(1);
    } 
    else {
        // Parent process: print job info, don't wait
        printf("[%d] %d\n", job_number, pid);
        fflush(stdout);
        return pid; // keep track of this background job
    }
}
/* End of part 8*/
