#ifndef TILDE_EXPANSION_H
#define TILDE_EXPANSION_H

// Function to expand ~ in a single token
// Returns new string with ~ expanded, caller must free()
char* expand_tilde(const char* token);

// Function to expand ~ in array of tokens  
// Modifies tokens array in place
void expand_tilde_in_tokens(char** tokens, int num_tokens);

#endif
