#include <stdio.h>
#include <stdlib.h>
#include <microhttpd.h>

int handle_request(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  struct MHD_Response *response;
  int ret;

  char *input_file = (char*)cls;
  FILE *fp = fopen(input_file, "rb");
  if (!fp) {
    return MHD_NO;
  }

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  char *mp3_data = malloc(fsize + 1);
  fread(mp3_data, fsize, 1, fp);
  fclose(fp);

  response = MHD_create_response_from_buffer(fsize, (void*)mp3_data, MHD_RESPMEM_MUST_FREE);
  MHD_add_response_header(response, "Content-Type", "audio/mpeg");

  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);

  return ret;
}

int main(int argc, char *argv[]) {
  struct MHD_Daemon *daemon;
  int port;

  if (argc != 3) {
    printf("Usage: %s <input_file.mp3> <port>\n", argv[0]);
    return 1;
  }

  char *input_file = argv[1];
  port = atoi(argv[2]);

  daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port, NULL, NULL, &handle_request, input_file, MHD_OPTION_END);
  if (!daemon) {
    return 1;
  }

  getchar();

  MHD_stop_daemon(daemon);
  return 0;
}

