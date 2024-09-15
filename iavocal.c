#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libwebsockets.h>
#include <jansson.h>
#include <curl/curl.h>

#define MAX_CHAT_ENTRIES 100
#define MAX_INPUT_LENGTH 1024
#define MAX_RESPONSE_LENGTH 4096
#define CONFIG_JSON "{\"config\": {\"sample_rate\":192000}}"
#define API_URL "http://chatgpt.free-solutions.ch:8080"
#define INSTRUCTION "Une conversation entre un humain curieux et un assistant d'intelligence artificielle. L'assistant donne des réponses utiles, détaillées et polies aux questions de l'humain."

char* chat[MAX_CHAT_ENTRIES * 2];
int chat_size = 0;
static FILE *audio_file = NULL;
static struct lws *global_wsi = NULL;

typedef struct { char* ptr; size_t len; } string_t;

void init_string(string_t* s) { s->len = 0; s->ptr = malloc(1); s->ptr[0] = '\0'; }

size_t writefunc(void* ptr, size_t size, size_t nmemb, string_t* s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return size * nmemb;
}

char* trim(char* str) {
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void format_prompt(char* prompt, size_t prompt_size, const char* question) {
    int written = snprintf(prompt, prompt_size, "%s\n", INSTRUCTION);
    size_t remaining = prompt_size - written;
    for (int i = 0; i < chat_size && remaining > 0; i += 2) {
        written = snprintf(prompt + strlen(prompt), remaining, "### Human: %s\n### Assistant: %s\n", chat[i], chat[i + 1]);
        remaining -= written;
    }
    if (remaining > 0) snprintf(prompt + strlen(prompt), remaining, "### Human: %s\n### Assistant: ", question);
}

int tokenize(const char* content) {
    CURL* curl = curl_easy_init();
    string_t s;
    init_string(&s);
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "%s/tokenize", API_URL);
        json_t* json = json_object();
        json_object_set_new(json, "content", json_string(content));
        char* json_str = json_dumps(json, 0);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        json_decref(json);
        free(json_str);
        curl_slist_free_all(headers);
    }
    json_t* parsed_json = json_loads(s.ptr, 0, NULL);
    json_t* tokens_array = json_object_get(parsed_json, "tokens");
    int token_count = json_array_size(tokens_array);
    free(s.ptr);
    json_decref(parsed_json);
    return token_count;
}

void chat_completion(const char* question, const char* tts_url, const char* speaker_id, const char* language_id) {
    CURL* curl = curl_easy_init();
    string_t s;
    init_string(&s);
    if (curl) {
        char url[256], prompt[MAX_INPUT_LENGTH * MAX_CHAT_ENTRIES];
        snprintf(url, sizeof(url), "%s/completion", API_URL);
        format_prompt(prompt, sizeof(prompt), question);
        int n_keep = tokenize(INSTRUCTION);
        json_t* json = json_object();
        json_object_set_new(json, "prompt", json_string(prompt));
        json_object_set_new(json, "temperature", json_real(0.8));
        json_object_set_new(json, "top_k", json_integer(40));
        json_object_set_new(json, "top_p", json_real(0.9));
        json_object_set_new(json, "n_keep", json_integer(n_keep));
        json_object_set_new(json, "n_predict", json_integer(400));
        json_object_set_new(json, "stream", json_boolean(1));
        json_t* stop_array = json_array();
        json_array_append_new(stop_array, json_string("\nHuman:"));
        json_array_append_new(stop_array, json_string("\n### Human:"));
        json_array_append_new(stop_array, json_string("Humain "));
        json_object_set_new(json, "stop", stop_array);
        char* json_str = json_dumps(json, 0);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        json_decref(json);
        free(json_str);
        curl_slist_free_all(headers);
    }
    char* token = strtok(s.ptr, "\n");
    char response[MAX_RESPONSE_LENGTH] = "";
    while (token != NULL) {
        if (strncmp(token, "data:", 5) == 0) {
            json_t* parsed_json = json_loads(token + 5, 0, NULL);
            json_t* content = json_object_get(parsed_json, "content");
            if (json_is_string(content)) {
                const char* content_str = json_string_value(content);
                printf("%s", content_str);
                fflush(stdout);
                strncat(response, content_str, MAX_RESPONSE_LENGTH - strlen(response) - 1);
            }
            json_decref(parsed_json);
        }
        token = strtok(NULL, "\n\n");
    }
    printf("\n");
    chat[chat_size++] = strdup(question);
    chat[chat_size++] = strdup(trim(response));
    free(s.ptr);

    // TTS part
    CURL *tts_curl = curl_easy_init();
    if (tts_curl) {
        size_t param_size = strlen(response) + strlen(speaker_id) + strlen(language_id) + 50;
        char* parameters = malloc(param_size);
        if (parameters) {
            snprintf(parameters, param_size, "text=%s&speaker_id=%s&language_id=%s", response, speaker_id, language_id);
            struct curl_slist *tts_headers = curl_slist_append(NULL, "Content-Type: application/x-www-form-urlencoded");
            curl_easy_setopt(tts_curl, CURLOPT_URL, tts_url);
            curl_easy_setopt(tts_curl, CURLOPT_POSTFIELDS, parameters);
            curl_easy_setopt(tts_curl, CURLOPT_HTTPHEADER, tts_headers);
            FILE *file = fopen("tts.wav", "wb");
            if (file) {
                curl_easy_setopt(tts_curl, CURLOPT_WRITEFUNCTION, fwrite);
                curl_easy_setopt(tts_curl, CURLOPT_WRITEDATA, file);
                curl_easy_perform(tts_curl);
                fclose(file);
                system("amixer -q -D pulse sset Capture toggle && aplay -q tts.wav && amixer -q -D pulse sset Capture toggle");
            }
            curl_slist_free_all(tts_headers);
            free(parameters);
        }
        curl_easy_cleanup(tts_curl);
    }
}

static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    unsigned char buf[LWS_PRE + 51200], *p = &buf[LWS_PRE];
    size_t n;
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("WebSocket connection established\n");
            global_wsi = wsi;
            n = sprintf((char *)p, "%s", CONFIG_JSON);
            if (lws_write(wsi, p, n, LWS_WRITE_TEXT) < 0) return -1;
            audio_file = popen("ffmpeg -f alsa -i default -ar 192000 -ac 1 -acodec pcm_s16le -hide_banner -loglevel quiet -nostats -f wav -", "r");
            if (!audio_file) return -1;
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            json_t *root = json_loads((const char *)in, 0, NULL);
            if (root) {
                json_t *text = json_object_get(root, "text");
                if (json_is_string(text)) {
                    const char *received_text = json_string_value(text);
                    if (strlen(received_text) > 0) {
                        printf("%s\n", received_text);
                        FILE *fichier = fopen("sortie.txt", "a");
                        if (fichier) {
                            fprintf(fichier, "%s\n", received_text);
                            fclose(fichier);
                        }
                        chat_completion(received_text, "http://chatgpt.free-solutions.ch:5006/api/tts", "female-en-5%0A", "fr-fr");
                    }
                }
                json_decref(root);
            }
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (audio_file) {
                n = fread(p, 1, sizeof(buf) - LWS_PRE, audio_file);
                if (n > 0) {
                    if (lws_write(wsi, p, n, LWS_WRITE_BINARY) < 0) return -1;
                    lws_callback_on_writable(wsi);
                } else {
                    pclose(audio_file);
                    audio_file = NULL;
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { "my-protocol", callback, 0, 51200, },
    { NULL, NULL, 0, 0 }
};

int main(int argc, char **argv) {
    struct lws_context_creation_info info = {0};
    struct lws_client_connect_info i = {0};
    const char *url = "chatgpt.free-solutions.ch";
    int port = 8081;

    chat[chat_size++] = strdup("Bonjour, Assistant.");
    chat[chat_size++] = strdup("Bonjour. Comment puis je vous aider aujourd'hui?");

    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE, NULL);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    struct lws_context *context = lws_create_context(&info);
    if (!context) return -1;

    i.context = context;
    i.address = url;
    i.port = port;
    i.path = "/streaming";
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = 0;
    i.protocol = protocols[0].name;

    if (!lws_client_connect_via_info(&i)) {
        lws_context_destroy(context);
        return -1;
    }

    while (lws_service(context, 1000) >= 0);

    lws_context_destroy(context);
    for (int i = 0; i < chat_size; i++) free(chat[i]);
    return 0;
}
