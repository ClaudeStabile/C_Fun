#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <ctype.h>

#define MAX_CHAT_ENTRIES 100
#define MAX_INPUT_LENGTH 1024
#define MAX_RESPONSE_LENGTH 4096

const char* API_URL = "http://chatgpt.free-solutions.ch:8080";
const char* INSTRUCTION = "Une conversation entre un humain curieux et un assistant d'intelligence artificielle. L'assistant donne des réponses utiles, détaillées et polies aux questions de l'humain.";

char* chat[MAX_CHAT_ENTRIES * 2];
int chat_size = 0;

struct string {
    char* ptr;
    size_t len;
};

void init_string(struct string* s) {
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

size_t writefunc(void* ptr, size_t size, size_t nmemb, struct string* s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

char* trim(char* str) {
    char* end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return str;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;
}

void format_prompt(char* prompt, const char* question) {
    sprintf(prompt, "%s\n", INSTRUCTION);
    for (int i = 0; i < chat_size; i += 2) {
        sprintf(prompt + strlen(prompt), "### Human: %s\n### Assistant: %s\n", chat[i], chat[i + 1]);
    }
    sprintf(prompt + strlen(prompt), "### Human: %s\n### Assistant: ", question);
}

int tokenize(const char* content) {
    CURL* curl;
    CURLcode res;
    struct string s;
    init_string(&s);

    curl = curl_easy_init();
    if (curl) {
        char url[256];
        sprintf(url, "%s/tokenize", API_URL);

        struct json_object* json = json_object_new_object();
        json_object_object_add(json, "content", json_object_new_string(content));
        const char* json_string = json_object_to_json_string(json);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        json_object_put(json);
        curl_slist_free_all(headers);
    }

    struct json_object* parsed_json = json_tokener_parse(s.ptr);
    struct json_object* tokens_array;
    json_object_object_get_ex(parsed_json, "tokens", &tokens_array);
    int token_count = json_object_array_length(tokens_array);

    free(s.ptr);
    json_object_put(parsed_json);

    return token_count;
}

void chat_completion(const char* question) {
    CURL* curl;
    CURLcode res;
    struct string s;
    init_string(&s);

    curl = curl_easy_init();
    if (curl) {
        char url[256];
        sprintf(url, "%s/completion", API_URL);

        char prompt[MAX_INPUT_LENGTH * MAX_CHAT_ENTRIES];
        format_prompt(prompt, question);

        int n_keep = tokenize(INSTRUCTION);

        struct json_object* json = json_object_new_object();
        json_object_object_add(json, "prompt", json_object_new_string(prompt));
        json_object_object_add(json, "temperature", json_object_new_double(0.8));
        json_object_object_add(json, "top_k", json_object_new_int(40));
        json_object_object_add(json, "top_p", json_object_new_double(0.9));
        json_object_object_add(json, "n_keep", json_object_new_int(n_keep));
        json_object_object_add(json, "n_predict", json_object_new_int(400));
        json_object_object_add(json, "stream", json_object_new_boolean(1));

        struct json_object* stop_array = json_object_new_array();
        json_object_array_add(stop_array, json_object_new_string("\nHuman:"));
        json_object_array_add(stop_array, json_object_new_string("\n### Human:"));
        json_object_array_add(stop_array, json_object_new_string("Humain "));
        json_object_object_add(json, "stop", stop_array);

        const char* json_string = json_object_to_json_string(json);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        json_object_put(json);
        curl_slist_free_all(headers);
    }

    char* token = strtok(s.ptr, "\n");
    char response[MAX_RESPONSE_LENGTH] = "";
    while (token != NULL) {
        if (strncmp(token, "data:", 5) == 0) {
            struct json_object* parsed_json = json_tokener_parse(token + 5);
            struct json_object* content;
            if (json_object_object_get_ex(parsed_json, "content", &content)) {
                const char* content_str = json_object_get_string(content);
                printf("%s", content_str);
                fflush(stdout);
                strcat(response, content_str);
            }
            json_object_put(parsed_json);
        }
        token = strtok(NULL, "\n\n");
    }
    printf("\n");

    chat[chat_size++] = strdup(question);
    chat[chat_size++] = strdup(trim(response));

    free(s.ptr);
}

int main() {
    chat[chat_size++] = strdup("Bonjour, Assistant.");
    chat[chat_size++] = strdup("Bonjour. Comment puis je vous aider aujourd'hui?");

    char input[MAX_INPUT_LENGTH];
    while (1) {
//        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = 0;
        chat_completion(input);
    }

    for (int i = 0; i < chat_size; i++) {
        free(chat[i]);
    }

    return 0;
}
