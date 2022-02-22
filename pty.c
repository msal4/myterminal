//
// Created by Mohammed Salman on 21/11/2021.
//

#include "pty.h"
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <util.h>

bool pty_open(PTY *pty) {
  if (openpty(&pty->master, &pty->slave, NULL, NULL, NULL) == -1) {
    perror("openpty");
    return false;
  }

  return true;
}

#define SHELL "/bin/zsh"

bool pty_spawn_shell(PTY *pty, const char **env) {
  setenv("TERM", "xterm-256color", true);

  pid_t pid = fork();
  if (pid == 0) {
    setsid();
    if (ioctl(pty->slave, TIOCSCTTY, NULL) == -1) {
      perror("ioctl");
      return false;
    }

    execle(SHELL, "+" SHELL, NULL, env);

    return false;
  } else if (pid > 0) {
    return true;
  }

  perror("fork");
  return false;
}
