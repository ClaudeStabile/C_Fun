#include <stdio.h>
#include <curl/curl.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <unistd.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    return fwrite(ptr, size, nmemb, stdout);
}

int main(int argc, char *argv[]) {
    CURL *curl;
    char *url;
    int refresh_rate = 5; // in seconds

    if (argc < 2) {
        printf("Usage: %s <rss_feed_url> [refresh_rate]\n", argv[0]);
        return 1;
    }
    url = argv[1];

    if (argc >= 3) {
        refresh_rate = atoi(argv[2]);
    }

    while (1) {
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        sleep(refresh_rate);
    }
    return 0;
}

