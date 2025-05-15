#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define sherror(error)                                                         \
  fprintf(stderr, "Caught an error on [%s: %d]\nerror: %s\n\n", __FILE__,      \
          __LINE__, error);

#define MAX_COMMAND_LENGTH 500
#define PARAM_LEN 50

// print the prompt icon& space to the user
void print_prompt() { printf("catalyst > "); }

// function to read the command from the user
int read_commands(char *command, char **params) {
  char *buf;

  buf = malloc(sizeof(char) * MAX_COMMAND_LENGTH);
  if (!buf) {
    sherror("allocation failed\n");
    exit(1);
  } else {
    memset(buf, 0, sizeof(int));
  }

  // get the input from the user and stream it into the buffer
  if (fgets(buf, MAX_COMMAND_LENGTH, stdin) == NULL) {
    perror("fgets failed\n");
    sherror("fgets expects a string to as an input\n");
    return -2;
  }

  // remove the newline
  size_t len = sizeof(buf);
  if (len > 0 && buf[len - 1] == '\n') {
    buf[len - 1] = '\0';
    len--;
  }

  // check if command is too long
  if (len >= MAX_COMMAND_LENGTH) {
    sherror("command too long\n");
    return -3;
  }

  // parse the input into command and parameters
  // eg: ls -la [ here ls is the command and -ls is the parameter passed to the
  // command ]
  // after the command there is a  space bwtweeen the command  & parameter

  int i = 0;
  char *token = strtok(buf, " ");
  while (token && i <= PARAM_LEN - 1) {
    params[i] = strdup(token);
    if (params[i] == NULL) {
      while (i-- > 0) {
        free(params[i]);
        free(buf);
        return -1;
      }
      i++;
      token = strtok(NULL, " ");
    }
    params[i] = NULL;
  }
  free(buf);
  return 0;
}

int main() { return 0; }
