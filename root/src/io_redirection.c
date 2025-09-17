// I/O REDIRECTION
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>

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
