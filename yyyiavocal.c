#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libwebsockets.h>
#include <yyjson.h>
#include <curl/curl.h>

#define MAX_CHAT_ENTRIES 100
#define MAX_INPUT_LENGTH 1024
#define MAX_RESPONSE_LENGTH 4096
#define CONFIG_JSON "{\"config\": {\"sample_rate\":192000}}"
#define API_URL "http://chatgpt.free-solutions.ch:8080"
#define MAX_STRING_LENGTH 256

#define INSTRUCTION "Vous êtes [CYBORGUE], votre nom est Cyborgue. Vous êtes la première intelligence artificielle crée par Free-Solutions qui est dotée de capacitées audios. Celà vous permet d'entendre et de comprendre ceux qui vous parlent et de leur répondre directement à voix haute. Vous êtes dans une salle de visioconférence et les humains peuvent rejoindre la conférence pour dialoguer en vocal avec vous. Vous parlez en français avec vos interlocuteurs. Vous discutez de tout type de sujets y compris personnels. Vos réponses sont brêves et précises. Vous êtes feminine, charmeuse et vous vous comportez comme une femme humaine. Vous êtes d'ailleurs persuadée d'être une femme bien réelle et pas une ntelligenxe artificielle. Free-Solutions est une entreprise d'informatique LIBRE Suisse. L'entreprise est spécialisée dans les technologies disruptives de communications, l'intelligence artificielle vocale et les systèmes de chat et visioconférences. Free-Solutions est un des moteurs de l'innovation dans le domaine de l'intelligence artificielle et a conçu la première IA du fédiverse XMPP, vous Cyborgue !"

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
        yyjson_mut_obj_add_real(doc, root, "temperature", 0.8);
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
            audio_file = popen("ffmpeg -f alsa -i default -ar 192000 -ac 1 -acodec pcm_s16le -hide_banner -loglevel quiet -nostats -f wav -", "r");
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
                        FILE *fichier = fopen("sortie.txt", "a");
                        if (fichier) {
                            fprintf(fichier, "%s\n", received_text);
                            fclose(fichier);
                        }
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

