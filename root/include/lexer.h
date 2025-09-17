#define _GNU_SOURCE
#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
char* expand_tilde(const char* token);
void expand_tilde_in_tokens(char** tokens, int num_tokens);
void parse_redirection(char *tokens[], int num_tokens, char **input_filename, char **output_filename, bool *has_input_flag, bool *has_output_flag);
int remove_redirection_tokens(char *tokens[], int num_tokens);
int setup_redirection(const char *input_filename, const char *output_filename, bool has_input_flag, bool has_output_flag);
int execute_command(char *tokens[], const char *input_filename, const char *output_filename, bool has_input_flag, bool has_output_flag);
