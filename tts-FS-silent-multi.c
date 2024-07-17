#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Define a callback function to write the response data to a file
size_t write_data_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <URL> <speaker_id> <language_id>\n", argv[0]);
        return 1;
    }

    const char *url = argv[1];
    const char *speaker_id = argv[2];
    const char *language_id = argv[3];

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return 1;
    }

    while (1) {
        char input_buffer[2048];
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            fprintf(stderr, "Failed to read input from stdin\n");
            break;
        }

        size_t input_len = strlen(input_buffer);
        if (input_len > 0 && input_buffer[input_len - 1] == '\n') {
            input_buffer[input_len - 1] = '\0';
        }

        if (strcasecmp(input_buffer, "exit") == 0) {
            break;
        }

        char parameters[4096];
        snprintf(parameters, sizeof(parameters), "text=%s&speaker_id=%s&language_id=%s", input_buffer, speaker_id, language_id);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        FILE *file = fopen("tts.wav", "wb");
        if (!file) {
            fprintf(stderr, "Failed to open file for writing\n");
            return 1;
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "HTTP request failed: %s\n", curl_easy_strerror(res));
        } else {
            fclose(file);

            // Play the generated tts.wav file using aplay
            system("amixer -q -D pulse sset Capture toggle");
            system("aplay -q tts.wav");
            system("amixer -q -D pulse sset Capture toggle");
            fflush(stdin);
        }

        curl_slist_free_all(headers);
    }

    curl_easy_cleanup(curl);

    return 0;
}

