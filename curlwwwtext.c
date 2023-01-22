#include <stdio.h>
#include <curl/curl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


int main( int argc, char *argv[])
{
	            // Boucle pour vérifier les paramètres
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Aide :\n");
            printf("Ce programme diffuse le fichier .webm passé en argument\n sur l'URL http://localhost:4848\n");
            printf("Help :\n");
            printf("This program stream .webm webm file passed as first argument\n to URL http://localhost:4848\n");
            return 0;
        }
    char *url;
    if (argc > 1) {
        url = argv[1];
    } else {
        printf("Aucun paramètre fourni\n");
        return 0;
    }
    printf("URL: %s\n", url);
    // }
    CURL *curl;
    CURLcode res;
    //char *url;
    char command[256];
    FILE *html;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        html = fopen("page.html", "w");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() a échoué: %s\n", curl_easy_strerror(res));

        curl_easy_cleanup(curl);
        fclose(html);
    }
    else {
        printf("Impossible d'initialiser CURL\n");
        exit(1);
    }
    // snprintf(command, sizeof(command), "html2text -ascii -nobs -style pretty page.html");
    snprintf(command, sizeof(command), "html2markdown  --no-wrap-links --ignore-tables --ignore-images --ignore-links page.html | grep -v '[[:digit:]]' | sed -E 's/###/Infos/g' | sed -E 's/##/Actus/g' | sed -E 's/#/Titre/g' | grep -v '* ' | grep -v 'Publicité'");
    system(command);
}
    return 0;
}

