#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define sherror(error)                                                         \
  fprintf(stderr, "Caught an error on [%s: %d]\nerror: %s\n\n", __FILE__,      \
          __LINE__, error);

#define MAX_COMMAND_LENGTH 500
#define PARAM_LEN 50

// print the prompt icon to the user
void print_prompt() {
  printf("catalyst > ");
  fflush(stdout); // Ensure the prompt is displayed
}

// function to read the command from the user
int read_commands(char **params) {
  char *buf;
  buf = malloc(sizeof(char) * MAX_COMMAND_LENGTH);
  if (!buf) {
    sherror("allocation failed\n");
    exit(1);
  } else {
    memset(buf, 0, MAX_COMMAND_LENGTH); // FIX: change to MAX_COMMAND_LENGTH
  }

  // get the input from the user and stream it into the buffer
  if (fgets(buf, MAX_COMMAND_LENGTH, stdin) == NULL) {
    perror("fgets failed\n");
    sherror("fgets expects a string to as an input\n");
    free(buf);
    return -2;
  }

  // remove the newline
  size_t len = strlen(buf); // Fix: sizeof(buf) was incorrect
  if (len > 0 && buf[len - 1] == '\n') {
    buf[len - 1] = '\0';
    len--;
  }

  // check if command is too long
  if (len >= MAX_COMMAND_LENGTH) {
    sherror("command too long\n");
    free(buf);
    return -3;
  }

  // parse the input into command and parameters
  int i = 0;
  char *token = strtok(buf, " ");

  // Clear any previous params
  for (int j = 0; j < PARAM_LEN; j++) {
    if (params[j] != NULL) {
      free(params[j]);
      params[j] = NULL;
    }
  }

  while (token && i < PARAM_LEN - 1) {
    params[i] = strdup(token);
    if (params[i] == NULL) {
      sherror("allocation failed for parameter\n");

      // free allocated params while (i-- > 0) { free(params[i]); }
      free(buf);
      return -1;
    }
    i++;
    token = strtok(NULL, " ");
  }

  params[i] = NULL; // mark the end of arguments
  free(buf);

  if (i == 0)
    return 1; // empty command
  return 0;
}

// make some  builtin commands
int custom_cd(char **params) {
  if (params[1] == NULL) {
    sherror("requires one or more arguement to cd\n");
    return 1;
  } else {
    if (chdir(params[1]) != 0) {
      perror("shell");
    }
  }
  return 1;
}

// ls command
int custom_ls() {

  DIR *d;
  struct dirnet *dir;
  d = opendir(".");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      printf("%s\n", *dir.d_name);
    }
    closedir(d);
  }
  return 1;
  return 1;
}

int main() {
  char *params[PARAM_LEN] = {NULL};
  int status;

  while (1) {
    print_prompt();
    status = read_commands(params);

    // handle command read errors
    if (status < 0) {
      continue;
    }

    // handle empty command
    if (status == 1 || params[0] == NULL) {
      continue;
    }

    if (strcmp(params[0], "exit") == 0) {
      break;
    }

    // execute the command
    pid_t pid = fork();

    if (pid == -1) {
      sherror("forking failed\n");
      continue;

    } else if (pid == 0) {

      execvp(params[0], params);

      perror("execvp failed");
      exit(1);

    } else {

      // parent process
      wait(NULL); // wait for child to finish
    }
  }

  // clean up any remaining allocated memory
  for (int i = 0; i < PARAM_LEN; i++) {
    if (params[i] != NULL) {
      free(params[i]);
    }
  }

  return 0;
}
