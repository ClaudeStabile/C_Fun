#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ENCODING_LENGTH 32

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    return fwrite(contents, size, nmemb, (FILE *)userp);
}

int detect_encoding(const char *filename, char *encoding, size_t encoding_size) {
    char command[MAX_COMMAND_LENGTH];
    FILE *pipe;

    snprintf(command, sizeof(command), "file -i %s | awk -F'charset=' '{print $2}'", filename);
    
    pipe = popen(command, "r");
    if (!pipe) {
        fprintf(stderr, "Error executing encoding detection command.\n");
        return -1;
    }

    if (fgets(encoding, encoding_size, pipe) == NULL) {
        fprintf(stderr, "Error reading encoding detection result.\n");
        pclose(pipe);
        return -1;
    }

    // Remove newline character if present
    encoding[strcspn(encoding, "\n")] = 0;

    pclose(pipe);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <URL> [--help]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        printf("Help:\n");
        printf("This program downloads the content from the given URL\n");
        printf("and converts it to a readable text format.\n");
        return 0;
    }

    const char *url = argv[1];
    printf("URL: %s\n", url);

    CURL *curl;
    CURLcode res;
    FILE *html_file;
    char command[MAX_COMMAND_LENGTH];
    char encoding[MAX_ENCODING_LENGTH];

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return 1;
    }

    html_file = fopen("page.html", "wb");
    if (!html_file) {
        fprintf(stderr, "Failed to open page.html for writing: %s\n", strerror(errno));
        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, html_file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    fclose(html_file);

    if (res == CURLE_OK) {
        if (detect_encoding("page.html", encoding, sizeof(encoding)) == 0) {
            printf("Detected encoding: %s\n", encoding);

            if (strcasecmp(encoding, "utf-8") != 0) {
                // Only convert if the encoding is not UTF-8
                snprintf(command, sizeof(command),
                         "iconv -f %s -t UTF-8 page.html | "
                         "html2markdown --no-wrap-links --ignore-tables --ignore-images --ignore-links | "
                         "grep -v '[[:digit:]]' | "
                         "sed -E 's/###/Infos/g' | sed -E 's/##/Actus/g' | sed -E 's/#/Titre/g' | "
                         "grep -v '* ' | grep -v 'Publicité'",
                         encoding);
            } else {
                // If it's already UTF-8, skip the iconv step
                snprintf(command, sizeof(command),
                         "html2markdown --no-wrap-links --ignore-tables --ignore-images --ignore-links page.html | "
                         "grep -v '[[:digit:]]' | "
                         "sed -E 's/###/Infos/g' | sed -E 's/##/Actus/g' | sed -E 's/#/Titre/g' | "
                         "grep -v '* ' | grep -v 'Publicité'");
            }

            int result = system(command);
            if (result == -1) {
                fprintf(stderr, "Failed to execute command: %s\n", strerror(errno));
                return 1;
            }
        } else {
            fprintf(stderr, "Failed to detect encoding\n");
            return 1;
        }
    }

    return 0;
}
