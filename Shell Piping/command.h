#ifndef _COMMAND_H
#define _COMMAND_H

typedef struct command_list {
  unsigned n;   // Number of commands.
  char ***cmd;  // Array of char**,
                // each cmd[i] is a command (an array of char*)
                // each cmd[i] ends with a (char*)NULL element
} command_list;

void chain_piping(const command_list *);

#endif
