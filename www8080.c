#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUF_SIZE 8192
#define DEFAULT_PORT 8080
int main(int argc, char *argv[]) {
    int PORT,socket_desc, client_sock, c, read_size;
    struct sockaddr_in server, client;
    char *message;
    FILE *file;

    // Set the port to the default if one wasn't provided
    if (argc < 2) {
        PORT = DEFAULT_PORT;
    } else {
        PORT = atoi(argv[1]);
    }

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        printf("Could not create socket");
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
        puts("bind failed");
        return 1;
    }

    // Listen
    listen(socket_desc, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while (1) {
        // Accept connection from an incoming client
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock < 0) {
            perror("accept failed");
            return 1;
        }
        puts("Connection accepted");

        // Read file content
        FILE *file = fopen("index.html", "r");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }
        fseek(file, 0, SEEK_END);
        long int size = ftell(file);
        rewind(file);
        message = (char *)malloc((size + 1) * sizeof(char));
        fread(message, sizeof(char), size, file);
        message[size] = '\0';
        fclose(file);

        // Send HTML content
        write(client_sock, "HTTP/1.1 200 OK\n", 16);
        write(client_sock, "Content-Type: text/html\n\n", 25);
        write(client_sock, message, strlen(message));

        // Close connection
        close(client_sock);
    }

    // Close socket
    close(socket_desc);
    return 0;
}
