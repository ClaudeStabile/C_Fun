#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <URL>\n", argv[0]);
    return 1;
  }

  CURL *curl;
  FILE *fp;
  char outfilename[FILENAME_MAX] = "output.html";
  char command[FILENAME_MAX + 100];

  curl = curl_easy_init();
  if (curl) {
    fp = fopen(outfilename,"wb");
    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
  }
  snprintf(command, sizeof(command), "wkhtmltopdf --no-background %s -", outfilename);
  system(command);
  remove(outfilename);
  return 0;
}

