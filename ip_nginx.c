#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <zlib.h>

#define MAX_IP_COUNT 100000

typedef struct {
    char ip[16];
    int count;

} ip_entry;

int compare_count(const void *a, const void *b) {
    return ((ip_entry *)b)->count - ((ip_entry *)a)->count;
}

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *ent;
    gzFile gzf;
    char buffer[1024];
    ip_entry ip_counts[MAX_IP_COUNT];
    int ip_count = 0;
        if (argc < 2) {
        printf("Usage: %s directory [--html|--text]\n", argv[0]);
        return 1;
    }

    int display_html = 0;
    int display_text = 0;
    if (argc >= 3) {
        if (strcmp(argv[2], "--html") == 0) {
            display_html = 1;
        } else if (strcmp(argv[2], "--text") == 0) {
            display_text = 1;
        } else {
            printf("Invalid option %s\n", argv[2]);
            return 1;
        }
    }
   // Out pour tests return 1;

    // Vérifier que le répertoire est fourni en argument
    if (argc < 2) {
        printf("Missing directory argument\n");
        return 1;
    }

    // Ouvrir le répertoire fourni en argument
    dir = opendir(argv[1]);
    if (!dir) {
        printf("Failed to open directory %s\n", argv[1]);
        return 1;
    }

    // Parcourir les fichiers du répertoire
    while ((ent = readdir(dir)) != NULL) {
        char *filename = ent->d_name;
        if (strncmp(filename, "access.log.", 11) == 0 && strstr(filename, ".gz")) {
            




// Ouvrir le fichier gz
            char path[256];
            sprintf(path, "%s/%s", argv[1], filename);
            gzf = gzopen(path, "r");
            if (!gzf) {
                printf("Failed to open file %s\n", path);
                continue;
            }

            // Lire les lignes du fichier gz
            while (gzgets(gzf, buffer, sizeof(buffer))) {
                char *ip = strtok(buffer, " ");
                if (!ip) {
                    continue;
                }

                // Rechercher l'adresse IP dans le tableau
                int i;
                for (i = 0; i < ip_count; i++) {
                    if (strcmp(ip_counts[i].ip, ip) == 0) {
                        // Incrémenter le compteur pour cette adresse IP
                        ip_counts[i].count++;
                        break;
                    }
                }
                if (i == ip_count) {
                    // Ajouter une nouvelle entrée pour cette adresse IP
                    if (ip_count == MAX_IP_COUNT) {
                        printf("Maximum IP count reached\n");
                        break;
                    }
                    strcpy(ip_counts[ip_count].ip, ip);
                    ip_counts[ip_count].count = 1;
                    ip_count++;
                }
            }

            // Fermer le fichier gz
            gzclose(gzf);
        }
    }

    // Trier le tableau par nombre d'accès en ordre décroissant
    qsort(ip_counts, ip_count, sizeof(ip_entry), compare_count);

    // Afficher les résultats en format HTML
 if (display_html) {
    printf("<table>\n");
    printf("<tr><th>Adresse IP</th><th>Nombre d'accès</th></tr>\n");
    for (int i = 0; i < ip_count; i++) {
       
        printf("<tr><td>%s</td><td>%d</td></tr>\n", ip_counts[i].ip, ip_counts[i].count);
    }
    printf("</table>\n");
            } else if(display_text) {
        for (int i = 0; i < ip_count; i++) {
            printf("%s: %d\n", ip_counts[i].ip, ip_counts[i].count);
        }
    } else {
        printf("Invalid option, use --html or --text\n");
    }


    return 0;
}
