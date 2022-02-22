//
// Created by Mohammed Salman on 21/11/2021.
//

#pragma once

#include <stdbool.h>

typedef struct PTY PTY;
struct PTY {
  int master, slave;
  char buffer[1024];
};

bool pty_open(PTY *pty);

bool pty_spawn_shell(PTY *pty, const char **env);
