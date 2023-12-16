#include "command.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define PIPE_R 0
#define PIPE_W 1

void chain_piping(const command_list *cs) {
    // Check for 0 commands
    if (cs->n == 0) {
        return;
    }

    // Initialize the pipes
    int pfd[cs->n-1][2];
    for (int i = 0; i < cs->n-1; i++) {
        if (pipe(pfd[i]) == -1) {
            perror("Pipe instantiation error...\n");
            exit(1);
        }
    }

    // Open for parallel commands to execute
    pid_t pid;
    for (int i = 0; i < cs->n; i++) {
        switch (pid = fork()) {
            // child process
            case (0):
                // Not first command then redirect input to pipe read
                if (i != 0 && dup2(pfd[i-1][PIPE_R], STDIN_FILENO) == -1) {
                    perror("dup2 error...\n");
                    exit(4);
                }

                // Not last command then redirect output to pipe write
                if (i != cs->n-1 && dup2(pfd[i][PIPE_W], STDOUT_FILENO) == -1) {
                    perror("dup2 error...\n");
                    exit(4);
                }

                // Closing the pipe fds
                for (int a = 0; a < cs->n-1; a++) {
                    close(pfd[a][PIPE_R]);
                    close(pfd[a][PIPE_W]);
                }

                // Executing the commands
                if (execvp(cs->cmd[i][0], cs->cmd[i]) == -1) {
                    perror("Execution error...\n"); // Error if execvp fails
                    exit(3);
                }
                break;                

            // error handling with the fork
            case (-1):
                perror("Fork issue...\n");
                exit(2);

            // parent process does nothing
            default:
                break;
        }
    }

    // Closing all the file descriptors
    for (int j = 0; j < cs->n-1; j++) {
        close(pfd[j][PIPE_R]);
        close(pfd[j][PIPE_W]);
    }

    // Waiting for the child processes to terminate
    for (int k = 0; k < cs->n; k++) {
        wait(NULL);
    }
}