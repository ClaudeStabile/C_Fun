#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	    // Boucle pour vérifier les paramètres
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Aide :\n");
            printf("Ce programme diffuse le fichier .webm passé en argument\n sur l'URL http://localhost:8080\n");
	    printf("Help :\n");
            printf("This program stream .webm webm file passed as first argument\n to URL http://localhost:8080\n");
            return 0;
        }
    char *file_name;
    if (argc > 1) {
        file_name = argv[1];
    } else {
        printf("Aucun paramètre fourni\n");
        return 0;
    }
    printf("Nom de fichier: %s\n", file_name);
    printf("connectez vous sur http://localhost:8080\n");
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *response = "HTTP/1.1 200 OK\nContent-Type: audio/webm\n\n";
    // char *file_name = "music.webm";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("In setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    // Boucle infinie pour rester actif en tâche de fond
    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        // Envoi des en-têtes HTTP
        send(new_socket, response, strlen(response), 0);

        // Boucle de lecture et envoi du fichier audio au client
	    int fd = open(file_name, O_RDONLY);
    while (1) {
        // Lecture des données du fichier
        valread = read(fd, buffer, 1024);
        if (valread == 0) {
            // Fin de fichier atteinte, sortie de la boucle
            break;
        }
        if (valread < 0) {
            // Erreur de lecture, traitement de l'erreur
            perror("Error reading file");
            exit(EXIT_FAILURE);
        }
        // Envoi des données lues au client
        send(new_socket, buffer, valread, 0);
    }
    // Fermeture du fichier
    //close(fd);
        //fermeture du fichier et retour au debut
      //  lseek(fd, 0, SEEK_SET);
      //  close(new_socket);
    }
    close(server_fd);
    return 0;
}
}

