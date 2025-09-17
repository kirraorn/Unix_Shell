#include <stdio.h> 
#include <stlib.h>
#include <string.h>

void replace_env_tokens(char **argv) 
{
  for (int i=0; argv[i] != NULL; i++)
    {
      if (argv[i][0] == '$' && argv[i][1] != '\0')
      {
        const char *val = getenv(argv[i] +1);
        free(argv[i]);
        argv[i] = strdup(val ? val : "");
      }
    }
      
}
