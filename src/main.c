#include <dirent.h>
#include <fcntl.h>
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
#define BUFFER_SIZE 1240

// function prototypes
void print_prompt();
int read_commands(char **params);
int custom_cd(char **params);
int custom_ls(int argc, char **agrv);
int custom_touch(char **params);

// print the prompt icon to the user
void print_prompt() {
  printf("catalyst > ");
  fflush(stdout); // print prompt immediately
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

  // skip the "cd" command itself (params[0])
  char *dir = params[1] ? params[1] : getenv("HOME");

  if (!dir) {
    fprintf(stderr, "No home directory found\n");
    return 0;
  }

  if (chdir(dir) != 0) {
    perror("cd");
    return 0;
  }

  return 1;
}

// ls command
int custom_ls(int argc, char **argv) {
  pid_t pid = fork();

  if (pid == -1) {
    sherror("forking failed in custom_ls");
    return 0;
  } else if (pid == 0) {

    char **ls_args =
        malloc(sizeof(char *) * (argc + 2)); // +1 for ls & +1 for NULL
    if (!ls_args) {
      sherror("allocation failed for ls_args");
      exit(1);
    }

    // First argument is "ls"
    ls_args[0] = strdup("ls");
    if (!ls_args[0]) {
      sherror("allocation failed for ls command");
      free(ls_args);
      exit(1);
    }

    // copy all arguments after the "ls" command
    for (int i = 1; i < argc; i++) {
      ls_args[i] = strdup(argv[i]);
      if (!ls_args[i]) {
        sherror("allocation failed for ls argument");

        // free all prev allocations
        for (int j = 0; j < i; j++) {
          free(ls_args[j]);
        }
        free(ls_args);
        exit(1);
      }
    }

    // NULL to terminate the argument list
    ls_args[argc] = NULL;

    // Execute ls
    execvp("ls", ls_args);

    // if execvp returns, it has failed
    perror("execvp failed for ls");

    // clean up if execvp fails
    for (int i = 0; i < argc; i++) {
      if (ls_args[i])
        free(ls_args[i]);
    }
    free(ls_args);
    exit(1);
  } else {

    int status;
    waitpid(pid, &status, 0);
    return 1;
  }
}

// custom touch command
int custom_touch(char **params) {

  if (params[1] == NULL) {
    fprintf(stderr, "missing file or operand\n");
    return 0;
  }

  pid_t touch_child_pid = fork();

  if (touch_child_pid == -1) {
    perror("fork (touch)\n");
    return 0;

  } else if (touch_child_pid == 0) {

    // this is the child process, and we gotta do something here to mimic the
    // touch command
    int fd = open(params[1], O_CREAT | O_WRONLY);
    if (fd == -1) {
      fprintf(stderr, "missing file or operand\n");
      return 0;
    }
  }
  return 0;
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
    } else if (strcmp(params[0], "cd") == 0) {
      custom_cd(params);
      continue;
    } else if (strcmp(params[0], "ls") == 0) {
      custom_ls(1, params);
      continue;
    } else if (strcmp(params[0], "touch") == 0) {
      custom_touch(params);
      continue;
    } else if (strcmp(params[0], "exit") == 0) {
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

      wait(NULL); // wait for child to finish
    }
  }

  for (int i = 0; i < PARAM_LEN; i++) {
    if (params[i] != NULL) {
      free(params[i]);
    }
  }

  return 0;
}
