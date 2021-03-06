#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#include "commands.h"
#include "built_in.h"

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512]) {
  struct single_command* com = (*commands);

  if (strcmp(com->argv[0], "exit") == 0) { //do_single_commands doesn't return 1, so i add this code.
    return 1;
  }

  if (n_commands == 1) {
    do_single_commands(com);
  }

  if (n_commands > 1) {
    do_pipe(n_commands, com);
  }
  return 0;
}

int do_single_commands(struct single_command (*commands)[512]) {
  int status;
  struct single_command* com = (*commands);
  assert(com->argc != 0);
  int ex;
  int built_in_pos = is_built_in_command(com->argv[0]);
  if (built_in_pos != -1) {
    if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
      if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
        fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
      }
    } else {
      fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
      return -1;
    }
  } else if (strcmp(com->argv[0], "") == 0) {
    return 0;
  } else {
      if(strcmp(com->argv[com->argc-1], "&") == 0) {
        com->argv[com->argc-1] = '\0';
        com->argc = com->argc - 1;
        
        setpgid(0, 0);
        tcsetpgrp(0, getpid());
        int pid = fork();
        if(pid == 0) {
          setpgid(0 , 0);
          setpgid(0 , getppid());
          execv(com->argv[0], com->argv);
          exit(-1);
        }
        printf("%d\n", pid);
      }
      else {
        int i = fork();
        if(i==0){
        ex = execv(com->argv[0], com->argv);
        } 
      }
      


      if(ex == -1) {
      fprintf(stderr, "%s: command not found\n", com->argv[0]);
      }

    return -1;
  }
}

int do_pipe(int n_commands, struct single_command (*com)) {
  
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
