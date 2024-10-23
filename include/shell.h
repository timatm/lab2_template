#ifndef SHELL_H
#define SHELL_H

#include "command.h"

int spawn_proc(int, int, struct cmd *, struct cmd_node *);
int fork_cmd_node(struct cmd *cmd);
void redirection(int in ,int out ,struct cmd_code *cmd);
void shell();

#endif
