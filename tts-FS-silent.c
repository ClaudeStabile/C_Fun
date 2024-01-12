#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Define a callback function to write the response data to a file
size_t write_data_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }

    CURL *curl;
    CURLcode res;

    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return 1;
    }

    // Set the URL for the HTTP request
    const char *url = argv[1];

    while (1) {
        // Read the text data from stdin
        char input_buffer[2048];
        // printf("Enter the text to send (or 'exit' to quit): ");
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            fprintf(stderr, "Failed to read input from stdin\n");
            break; // Exit the loop on input error
        }

        // Remove the newline character if present
        size_t input_len = strlen(input_buffer);
        if (input_len > 0 && input_buffer[input_len - 1] == '\n') {
            input_buffer[input_len - 1] = '\0';
        }

        // Check for the exit command
        if (strcasecmp(input_buffer, "exit") == 0) {
            break; // Exit the loop
        }

        // Set the text as POST data
        char text[2058]; // Increased buffer size
        snprintf(text, sizeof(text), "text=%s", input_buffer);

        // Set the HTTP headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        // Set the HTTP request options
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the write callback function to write the response to a file
        FILE *file = fopen("tts.wav", "wb"); // Open the file for writing
        if (!file) {
            fprintf(stderr, "Failed to open file for writing\n");
            return 1;
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        // Perform the HTTP request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "HTTP request failed: %s\n", curl_easy_strerror(res));
        } else {
            fclose(file); // Close the file

            // Play the generated tts.wav file using aplay
            system("aplay -q tts.wav");
            fflush(stdout);
        }

        // Clean up and release resources
        curl_slist_free_all(headers);
    }

    curl_easy_cleanup(curl);

    return 0;
}

