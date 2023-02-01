#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s port\n", argv[0]);
        return 1;
    }

    char* port = argv[1];
    char line[256];
    FILE* logfile = fopen("ufw.log", "r");

    while (fgets(line, sizeof(line), logfile)) {
        if (strstr(line, port) != NULL) {
            char* ip = strstr(line, "SRC=") + 4;
            char* end = strstr(ip, " ");
            *end = '\0';
            printf("%s\n", ip);
        }
    }

    fclose(logfile);
    return 0;
}

