#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [arg1 arg2 ...]\n", argv[0]);
        return 1;
    }

    // Concatenate all the arguments into a single string
    char command[1024] = "";
    for (int i = 1; i < argc; i++) {
        strcat(command, argv[i]);
        strcat(command, " ");
    }

    pid_t pid = fork();  // Create a child process

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    if (pid == 0) {  // This block will be executed by the child process
        char *args[] = {"/bin/bash", "-c", command, NULL};
        execvp(args[0], args);  // Replace the current process image with a new one

        // If execvp returns, it must have failed.
        fprintf(stderr, "Exec failed\n");
        return 1;
    }

    // The parent process continues execution here.
//    printf("Started the command in the background, PID = %d\n", pid);

    return 0;
}

