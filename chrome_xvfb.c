#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int pid;
    char *url;

    if (argc < 2) {
        printf("Usage: %s <url>\n", argv[0]);
        return 1;
    }

    url = argv[1];

    // Start xvfb in the background
    pid = fork();
    if (pid == 0) {
        execlp("xvfb-run", "xvfb-run", "-a", "-s", "-screen 0 1280x720x24", "--", "google-chrome", "--disable-gpu", url, (char *)NULL);
    }

    // Wait for the browser to start
    sleep(5);

    return 0;
}

