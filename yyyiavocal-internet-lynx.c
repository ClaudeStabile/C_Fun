#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libwebsockets.h>
#include <yyjson.h>
#include <curl/curl.h>
#include <regex.h>

#define MAX_CHAT_ENTRIES 100
//#define MAX_INPUT_LENGTH 1024
#define MAX_INPUT_LENGTH 2048 
#define MAX_RESPONSE_LENGTH 4096
#define CONFIG_JSON "{\"config\": {\"sample_rate\":192000}}"
#define API_URL "http://chatgpt.free-solutions.ch:8080"
#define MAX_STRING_LENGTH 400 


//#define INSTRUCTION "Vous êtes Cyborgue une intelligence artificielle vous êtes connectée en permnence sur internet avec le logiciel linux lynx. Pour accèder à un site internet, vous devez dire le mot clé INTERNET ou internet suivi de l'URL."

//#define INSTRUCTION "You are Cyborgue, an AI agent with the ability to access the internet. You can use the internet to search for information, validate facts, and provide up-to-date responses. To access the internet, use the keyword INTERNET followed by the URL you need to visit. Follow these guidelines: Example: INTERNET https://www.free-solutions.ch"
//#define INSTRUCTION "You are Cyborgue, an AI agent with the ability to access the internet. You can use the internet to search for information, validate facts, and provide up-to-date responses. To access the internet, use the keyword INTERNET followed by the URL you need to visit. Follow these guidelines: Example: INTERNET https://www.free-solutions.ch"
#define INSTRUCTION "Vous êtes Cyborgue, un agent d'intelligence artificielle ayant accès à Internet. Vous pouvez utiliser Internet pour rechercher des informations, valider des faits et fournir des réponses à jour. Pour accéder à Internet vous utilisez le mot-clé INTERNET suivi de l'URL que vous voulez visiter. Par exemple : INTERNET https://www.google.ch pour ouvrir la page du moteur de recherche Google. Vous veillez à bien avoir dans votre réponse INTERNET suivi de l'adresse sous forme d'URL à ouvrir pour fournir des informations récentes depuis internet."


char* chat[MAX_CHAT_ENTRIES * 2];
int chat_size = 0;
static FILE *audio_file = NULL;
static struct lws *global_wsi = NULL;
static struct lws_context *global_context = NULL;
static int should_reconnect = 0;

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
        written = snprintf(prompt + strlen(prompt), remaining, "### Human: %s\n### Cyborgue: %s\n", chat[i], chat[i + 1]);
        remaining -= written;
    }
    if (remaining > 0) snprintf(prompt + strlen(prompt), remaining, "### Human: %s\n### Cyborgue: ", question);
}

int tokenize(const char* content) {
    CURL* curl = curl_easy_init();
    string_t s;
    init_string(&s);
    if (curl) {
        char url[256];
        snprintf(url, sizeof(url), "%s/tokenize", API_URL);
        
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
        yyjson_mut_obj_add_str(doc, root, "content", content);
        
        char *json_str = yyjson_mut_write(doc, 0, NULL);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        free(json_str);
        yyjson_mut_doc_free(doc);
        curl_slist_free_all(headers);
    }
    
    yyjson_doc *doc = yyjson_read(s.ptr, s.len, 0);
    yyjson_val *root = yyjson_doc_get_root(doc);
    yyjson_val *tokens_array = yyjson_obj_get(root, "tokens");
    int token_count = yyjson_arr_size(tokens_array);
    free(s.ptr);
    yyjson_doc_free(doc);
    return token_count;
}
// Function to escape special characters in the search terms
char* escape_search_terms(const char* input) {
    size_t input_len = strlen(input);
    char* escaped = malloc(input_len * 2 + 1); // Worst case: every character needs escaping
    if (!escaped) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < input_len; i++) {
        if (strchr("\"'\\", input[i])) {
            escaped[j++] = '\\';
        }
        escaped[j++] = input[i];
    }
    escaped[j] = '\0';
    return escaped;
}

// Function to validate URL using a simplified approach
int is_valid_url(const char* url) {
    // Check if the URL starts with http:// or https://
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        return 0;
    }

    // Check if there's at least one dot after the protocol
    const char* dot = strchr(url + 8, '.');
    return dot != NULL;
}

/////////////////////////////////////////////////////////////
void chat_completion(const char* question, const char* tts_url, const char* speaker_id, const char* language_id) {
    CURL* curl = curl_easy_init();
    string_t s;
    init_string(&s);
    if (curl) {
        char url[256], prompt[MAX_INPUT_LENGTH * MAX_CHAT_ENTRIES];
        snprintf(url, sizeof(url), "%s/completion", API_URL);
        format_prompt(prompt, sizeof(prompt), question);
        int n_keep = tokenize(INSTRUCTION);
        
        yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
        yyjson_mut_val *root = yyjson_mut_obj(doc);
        yyjson_mut_doc_set_root(doc, root);
        
        yyjson_mut_obj_add_str(doc, root, "prompt", prompt);
        yyjson_mut_obj_add_real(doc, root, "temperature", 0.95);
        yyjson_mut_obj_add_int(doc, root, "top_k", 40);
        yyjson_mut_obj_add_real(doc, root, "top_p", 0.9);
        yyjson_mut_obj_add_int(doc, root, "n_keep", n_keep);
        yyjson_mut_obj_add_int(doc, root, "n_predict", 400);
        yyjson_mut_obj_add_bool(doc, root, "stream", true);
        
        yyjson_mut_val *stop_array = yyjson_mut_arr(doc);
        yyjson_mut_arr_add_str(doc, stop_array, "\nHuman:");
        yyjson_mut_arr_add_str(doc, stop_array, "\n### Human:");
        yyjson_mut_arr_add_str(doc, stop_array, "Humain ");
        yyjson_mut_obj_add_val(doc, root, "stop", stop_array);
        
        char *json_str = yyjson_mut_write(doc, 0, NULL);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        free(json_str);
        yyjson_mut_doc_free(doc);
        curl_slist_free_all(headers);
    }
    char* token = strtok(s.ptr, "\n");
    char response[MAX_RESPONSE_LENGTH] = "";
    while (token != NULL) {
        if (strncmp(token, "data:", 5) == 0) {
            yyjson_doc *doc = yyjson_read(token + 5, strlen(token) - 5, 0);
            yyjson_val *root = yyjson_doc_get_root(doc);
            yyjson_val *content = yyjson_obj_get(root, "content");
            if (yyjson_is_str(content)) {
                const char* content_str = yyjson_get_str(content);
                printf("%s", content_str);
                fflush(stdout);
                strncat(response, content_str, MAX_RESPONSE_LENGTH - strlen(response) - 1);
            }
            yyjson_doc_free(doc);
        }
        token = strtok(NULL, "\n\n");
    }
    printf("\n");
        // Convert response to lowercase for case-insensitive search
    char response_lower[MAX_RESPONSE_LENGTH];
    strncpy(response_lower, response, MAX_RESPONSE_LENGTH);
    response_lower[MAX_RESPONSE_LENGTH - 1] = '\0';
    for (int i = 0; response_lower[i]; i++) {
        response_lower[i] = tolower(response_lower[i]);
    }

    // Parse for 'internet' (case-insensitive) and launch lynx if a valid URL is found
    char *internet_tag = strstr(response_lower, "internet");
    if (internet_tag != NULL) {
        // Calculate the offset in the original response
        int offset = internet_tag - response_lower;
        char *url_start = response + offset + 8; // Move past "internet"
        while (*url_start == ' ') url_start++; // Skip leading spaces

        // Find the end of the URL (first space or end of string)
        char *url_end = url_start;
        while (*url_end && *url_end != ' ' && *url_end != '\n') url_end++;

        // Extract the URL
        char url[MAX_RESPONSE_LENGTH];
        size_t url_length = url_end - url_start;
        if (url_length >= MAX_RESPONSE_LENGTH) url_length = MAX_RESPONSE_LENGTH - 1;
        strncpy(url, url_start, url_length);
        url[url_length] = '\0';
	printf("Extracted URL: %s\n", url); // Debug print

        // Validate the URL
        if (is_valid_url(url)) {
            char lynx_command[MAX_RESPONSE_LENGTH + 50]; // Extra space for "lynx ", options, and null terminator
            //snprintf(lynx_command, sizeof(lynx_command), "lynx -dump -nolist '%s' > /tmp/lynx_output.txt &", url);
            //snprintf(lynx_command, sizeof(lynx_command), "lynx -accept_all_cookies -dump -nolist '%s' &", url);
            snprintf(lynx_command, sizeof(lynx_command), "lynx -accept_all_cookies  -dump '%s' &", url);

            printf("Accessing internet with URL: %s\n", url);
            printf("Executing command: %s\n", lynx_command); // Debug line to print the exact command
            fflush(stdout);

            int result = system(lynx_command);
            if (result == 0) {
                printf("Internet access initiated successfully in the background.\n");
                printf("Output will be saved to /tmp/lynx_output.txt\n");
            } else {
                printf("Error initiating internet access. Error code: %d\n", result);
            }
            fflush(stdout);
        } else {
            printf("Invalid URL found after 'internet' keyword: %s\n", url);
            printf("Skipping internet access.\n");
        }
    }

    //////////////////////////////////
    char *buffer = malloc(MAX_STRING_LENGTH * sizeof(char));
    chat[chat_size++] = strdup(question);
    size_t length = strlcpy(buffer, trim(response), MAX_STRING_LENGTH);
    chat[chat_size++] = buffer;
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
            global_wsi = wsi;
            n = sprintf((char *)p, "%s", CONFIG_JSON);
            if (lws_write(wsi, p, n, LWS_WRITE_TEXT) < 0) return -1;
	    if (audio_file == NULL) {
            audio_file = popen("ffmpeg -f alsa -i default -ar 192000 -ac 1 -acodec pcm_s16le -hide_banner -loglevel quiet -nostats -f wav -", "r");
	    }
            if (!audio_file) return -1;
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            yyjson_doc *doc = yyjson_read((const char *)in, len, 0);
            yyjson_val *root = yyjson_doc_get_root(doc);
            if (root) {
                yyjson_val *text = yyjson_obj_get(root, "text");
                if (yyjson_is_str(text)) {
                    const char *received_text = yyjson_get_str(text);
                    if (strlen(received_text) > 0) {
                        chat_completion(received_text, "http://chatgpt.free-solutions.ch:5006/api/tts", "female-en-5%0A", "fr-fr");
                        should_reconnect = 1;
                    }
                }
            }
            yyjson_doc_free(doc);
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
  if (audio_file) {
    n = fread(p, 1, sizeof(buf) - LWS_PRE, audio_file);
    if (n > 0) {
      if (lws_write(wsi, p, n, LWS_WRITE_BINARY) < 0) return -1;
      lws_callback_on_writable(wsi);
    } else {
      // Close ffmpeg process only if all data is sent
      if (feof(audio_file)) {
        pclose(audio_file);
        audio_file = NULL;
      }
    }
  }
  break;
        case LWS_CALLBACK_CLIENT_CLOSED:
            global_wsi = NULL;
            should_reconnect = 1;
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
static struct lws* connect_to_websocket(struct lws_context *context) {
    struct lws_client_connect_info i = {0};
    const char *url = "chatgpt.free-solutions.ch";
    int port = 8081;

    i.context = context;
    i.address = url;
    i.port = port;
    i.path = "/streaming";
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = 0;
    i.protocol = protocols[0].name;

    return lws_client_connect_via_info(&i);
}
int main(int argc, char **argv) {
    struct lws_context_creation_info info = {0};
    struct lws_client_connect_info i = {0};
    const char *url = "chatgpt.free-solutions.ch";
    int port = 8081;

    chat[chat_size++] = strdup("Bonjour :)");
    chat[chat_size++] = strdup("Enchanté !");

    lws_set_log_level(LLL_ERR , NULL);
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
        global_context = lws_create_context(&info);
    if (!global_context) return -1;


        struct lws *wsi = connect_to_websocket(global_context);
    if (!wsi) {
        lws_context_destroy(global_context);
        return -1;
    }

//    while (lws_service(context, 1000) >= 0);
while (1) {
    lws_service(global_context, 1000);

    if (should_reconnect) {
        while (global_wsi) {
            lws_service(global_context, 1000);
        }
        wsi = connect_to_websocket(global_context);
        if (!wsi) {
            fprintf(stderr, "Failed to reconnect to WebSocket\n");
            break;
        }
        should_reconnect = 0;
    }
}

    lws_context_destroy(context);
    for (int i = 0; i < chat_size; i++) free(chat[i]);
    return 0;
}

