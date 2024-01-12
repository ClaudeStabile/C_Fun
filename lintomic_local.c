#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>
#include <jansson.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define AUDIO_FILE_PATH "audio.wav"
#define CONFIG_JSON "{\"config\": {\"sample_rate\":192000}}"

static int callback(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len)
{
    unsigned char buf[LWS_PRE + 51200];
    unsigned char *p = &buf[LWS_PRE];
    static FILE *audio_file;
    size_t n;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:

            // Send config JSON
            n = sprintf((char *)p, "%s", CONFIG_JSON);
            lws_write(wsi, p, n, LWS_WRITE_TEXT);

            // Open audio file
        audio_file = popen("ffmpeg -f alsa -i default -ar 192000 -ac 1 -acodec pcm_s16le -hide_banner -loglevel quiet -nostats -f wav -", "r");
            if (audio_file == NULL) {
                fprintf(stderr, "Failed to open audio file\n");
                return -1;
            }
            break;

            case LWS_CALLBACK_CLIENT_RECEIVE:
    json_t *root, *text;
    json_error_t error;

    root = json_loads((const char *)in, 0, &error);
    if (!root) {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return -1;
    }

    // Variable pour l'écriture dans un fichiet d output
    //
    FILE *fichier;
    const char *nom_fichier = "sortie.txt";
//    const char *texte = "Texte à écrire dans le fichier";

    text = json_object_get(root, "text");
    if (json_is_string(text)) {
	    if (strlen(json_string_value(text)) > 0) {
            fprintf(stdout, "%s\n", json_string_value(text));
            fichier = fopen(nom_fichier, "a");
            fprintf(fichier, "%s\n", json_string_value(text));
            fflush(fichier);
            fclose(fichier);
            fflush(stdout);

	    }
    if (fichier == NULL) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", nom_fichier);
        return 1;
    }


    }


    json_decref(root);
    break;


        case LWS_CALLBACK_CLIENT_WRITEABLE:
            // Send audio data
            if (audio_file != NULL) {
                n = fread(p, 1, sizeof(buf) - LWS_PRE, audio_file);
                if (n > 0) {
                    lws_write(wsi, p, n, LWS_WRITE_BINARY);
                } else {
                    fclose(audio_file);
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
    {
        "my-protocol",
        callback,
        0,
        0,
    },
    { NULL, NULL, 0, 0 } /* terminator */
};

int main(int argc, char **argv)
{
    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: %s \n\nLancez simplement l'executable pour capturer live l'audio de votre micro et le transcrire en texte FR dans votre terminal\nCe programme utilise l'engine speech to text LINTO AI https://github.com/linto-ai/\nCe programme utilise le backend free-solutions.org en Suisse pour transcrire le text\nC'est une approche pour avoir le contrôle total sur les transcrptions texte\nC'est expérimental et donc merci de ne pas utiliser pour un service\nMe contacter si vous avez des besoins dans le domaine\n", argv[0]);
        printf("Options:\n");
        printf("  -h, --help    display this help and exit\n");
    return 0;
    }
    struct lws_context_creation_info info;
    struct lws_client_connect_info i;
    struct lws_context *context;
    const char *url = "chatgpt.free-solutions.ch";
//    const char *url = "wss://localhost";
    int port = 8081;

// Adons CST
//lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO, NULL);
    info.ssl_ca_filepath = "/etc/ssl/certs/ca-certificates.crt";
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    memset(&info, 0, sizeof info);
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);
    if (context == NULL) {
        fprintf(stderr, "Failed to create context\n");
        return -1;
    }

    memset(&i, 0, sizeof(i));
    i.context = context;
    i.address = url;
    i.port = port;
    i.path = "/streaming";
    i.host = i.address;
    i.origin = i.address;
    i.ssl_connection = 0; // No SSL
//    i.ssl_connection = LCCSCF_USE_SSL; // Use SSL
    i.protocol = protocols[0].name;

    if (lws_client_connect_via_info(&i) == NULL) {
        fprintf(stderr, "Failed to connect to server\n");
        return -1;
    }

    while (1) {
        lws_service(context, 1000);
        lws_callback_on_writable_all_protocol(context,&protocols[0]);
        
    }

    lws_context_destroy(context);

    return 0;
}
