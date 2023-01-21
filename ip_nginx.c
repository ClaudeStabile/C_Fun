#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1000
#define MAX_IP_ADDRESSES 10000

struct IP {
    char address[20];
    int count;
} IPs[MAX_IP_ADDRESSES];

int num_ips = 0;

void process_line(char* line) {
    // Extraire l'adresse IP de la ligne
    char* token = strtok(line, " ");
    char* ip_address = token;

    // Rechercher si l'adresse IP existe déjà dans le tableau
    int ip_index = -1;
    for (int i = 0; i < num_ips; i++) {
        if (strcmp(IPs[i].address, ip_address) == 0) {
            ip_index = i;
            break;
        }
    }

    // Si l'adresse IP n'existe pas, l'ajouter au tableau
    if (ip_index == -1) {
        strcpy(IPs[num_ips].address, ip_address);
        IPs[num_ips].count = 1;
        num_ips++;
    }
    // Sinon, incrémenter le compteur pour cette adresse IP
    else {
        IPs[ip_index].count++;
    }
}

int cmp(const void* a, const void* b) {
    struct IP* ip1 = (struct IP*) a;
    struct IP* ip2 = (struct IP*) b;
    return ip2->count - ip1->count;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Utilisation : %s <fichier de log>\n", argv[0]);
        return 1;
    }

    // Ouvrir le fichier de log
    char* log_file = argv[1];
    FILE* fp = fopen(log_file, "r");
    if (fp == NULL) {
        printf("Impossible d'ouvrir le fichier %s\n", log_file);
        return 1;
    }

    // Traiter chaque ligne du fichier
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        process_line(line);
    }

    // Trier le tableau d'adresses IP par nombre de connexions décroissant
    qsort(IPs, num_ips, sizeof(struct IP), cmp);

    // Générer le tableau HTML
    printf("<table>\n");
    printf("<tr><th>Adresse IP</th><th>Nombre de connexions</th></tr>\n");
for (int i = 0; i < num_ips; i++) {
printf("<tr><td>%s</td><td>%d</td></tr>\n", IPs[i].address, IPs[i].count);
}
printf("</table>\n");
fclose(fp);
return 0;
}
